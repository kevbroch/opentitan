// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

//
// Base class for all OTBN test sequences
//

class otbn_base_vseq extends cip_base_vseq #(
    .CFG_T               (otbn_env_cfg),
    .RAL_T               (otbn_reg_block),
    .COV_T               (otbn_env_cov),
    .VIRTUAL_SEQUENCER_T (otbn_virtual_sequencer)
  );
  `uvm_object_utils(otbn_base_vseq)
  `uvm_object_new

  // "Running" flag to detect concurrent executions of run_otbn()
  protected bit running_ = 1'b0;

  // The longest run seen in calls to start_running_otbn. This gets reset to zero by load_elf (since
  // we assume we've got a new program, which might take a different amount of time)
  protected int unsigned longest_run_ = 0;

  // Load the contents of an ELF file into the DUT's memories, either by a DPI backdoor (if backdoor
  // is true) or with TL transactions. Also, pass loop warp rules to the ISS through the model.
  protected task load_elf(string path, bit backdoor);
    if (backdoor) begin
      load_elf_backdoor(path);
    end else begin
      load_elf_over_bus(path);
    end

    // Pass loop warp rules that we've just loaded into otbn_memutil into the model.
    otbn_take_loop_warps(cfg.model_agent_cfg.vif.handle, cfg.mem_util);

    // We're loading a new program, so the tracking that we've been doing for how long runs take is
    // no longer valid.
    longest_run_ = 0;
  endtask

  // Load the contents of an ELF file into the DUT's memories by a DPI backdoor
  protected function void load_elf_backdoor(string path);
    if (!OtbnMemUtilLoadElf(cfg.mem_util, path)) begin
      `uvm_fatal(`gfn, $sformatf("Failed to load ELF at `%0s'", path))
    end
  endfunction

  // Load the contents of an ELF file into the DUT's memories by TL transactions
  protected task load_elf_over_bus(string path);
    otbn_loaded_word to_load[$];

    // First, tell OtbnMemUtil to stage the ELF. This reads the file and stashes away the segments
    // we need. If something goes wrong, it will print a message to stderr, so we can just fail.
    if (!OtbnMemUtilStageElf(cfg.mem_util, path)) begin
      `uvm_fatal(`gfn, $sformatf("Failed to stage ELF at `%0s'", path))
    end

    // Next, we need to get the data to be loaded across the "DPI barrier" and into SystemVerilog.
    // We make a queue of the things that need loading (in address order) and then shuffle it, so
    // that we load the memory in an arbitrary order
    get_queue_entries(1'b0, to_load);
    get_queue_entries(1'b1, to_load);
    to_load.shuffle();

    // Send the writes, one by one
    foreach (to_load[i]) begin
      csr_utils_pkg::mem_wr(to_load[i].for_imem ? ral.imem : ral.dmem,
                            to_load[i].offset,
                            to_load[i].data);
    end
  endtask

  protected function automatic void
  get_queue_entries(bit for_imem, ref otbn_loaded_word entries[$]);
    // Get the size of this memory (to make sure the number of loaded words makes sense)
    int unsigned mem_size = for_imem ? OTBN_IMEM_SIZE : OTBN_DMEM_SIZE;

    // Iterate over the segments for this memory
    int seg_count = OtbnMemUtilGetSegCount(cfg.mem_util, for_imem);
    for (int seg_idx = 0; seg_idx < seg_count; seg_idx++) begin

      // What offset and size (in 32 bit words) is this segment?
      bit [31:0] seg_off, seg_size;
      if (!OtbnMemUtilGetSegInfo(cfg.mem_util, for_imem, seg_idx, seg_off, seg_size)) begin
        `uvm_fatal(`gfn, $sformatf("Failed to get segment info for segment %0d.", seg_idx))
      end

      // Add each word.
      for (bit [31:0] i = 0; i < seg_size; i++) begin
        bit [31:0] word_off, data;
        otbn_loaded_word entry;

        word_off = seg_off + i;

        if (!OtbnMemUtilGetSegData(cfg.mem_util, for_imem, word_off, data)) begin
          `uvm_fatal(`gfn, $sformatf("Failed to get segment data at word offset %0d.", word_off))
        end

        // Since we know that the segment data lies in IMEM or DMEM and that this fits in the
        // address space, we know that the top two bits of the word address are zero.
        `DV_CHECK_FATAL(word_off[31:30] == 2'b00)

        // OtbnMemUtil should have checked that this address was valid for the given memory, but it
        // can't hurt to check again.
        `DV_CHECK_FATAL({word_off, 2'b00} < {2'b00, mem_size})

        entry.for_imem = for_imem;
        entry.offset   = word_off[21:0];
        entry.data     = data;
        entries.push_back(entry);
      end
    end
  endfunction

  // Start OTBN and then wait until done
  //
  // If the block gets reset, this task will exit early.
  protected task run_otbn(input check_end_addr = 1);
    int exp_end_addr;

    // Check that we haven't been called re-entrantly. This could happen if there's a bug in the
    // reset sequence, which relies on run_otbn() to exit properly when it sees a device reset.
    // Explode here if that happens, which should be easier to debug than two concurrent run_otbn()
    // processes fighting over the interface.
    `DV_CHECK_FATAL(!running_)
    running_ = 1'b1;

    fork : isolation_fork
      begin
        fork
          _run_otbn();
          _run_loop_warps();
        join_any
        disable fork;
      end
    join

    // Post-run checks
    //
    // The CSR operations above short-circuit and exit immediately if the reset line goes low. If
    // that happens, we don't want to run the checks (since the run didn't finish properly).
    if (!cfg.under_reset && check_end_addr) begin
      // If there was an expected end address, compare it with the model. This isn't really a test of
      // the RTL, but it's handy to make sure that the RIG really is generating the control flow that
      // it expects.
      exp_end_addr = OtbnMemUtilGetExpEndAddr(cfg.mem_util);
      if (exp_end_addr >= 0) begin
        `DV_CHECK_EQ_FATAL(exp_end_addr, cfg.model_agent_cfg.vif.stop_pc)
      end
    end

    running_ = 1'b0;
  endtask

  // The guts of the run_otbn task. Writes to the CMD register to start OTBN and polls the status
  // register until completion. On reset, this returns immediately.
  protected task _run_otbn();
    // Start OTBN by writing EXECUTE to the CMD register.
    `uvm_info(`gfn, "\n\t ----| Starting OTBN", UVM_MEDIUM)
    csr_utils_pkg::csr_wr(ral.cmd, otbn_pkg::CmdExecute);

    // Wait for OTBN to finish, either by polling or by waiting on the interrupt pins
    if (_pick_use_interrupt()) begin
      `uvm_info(`gfn, "\n\t ----| Waiting for OTBN to finish (interrupt)", UVM_MEDIUM)
      wait_for_interrupt();
    end else begin
      `uvm_info(`gfn, "\n\t ----| Waiting for OTBN to finish (polling)", UVM_MEDIUM)
      csr_utils_pkg::csr_spinwait(.ptr(ral.status),
                                  .exp_data(otbn_pkg::StatusBusyExecute),
                                  .compare_op(CompareOpNe));
    end

    if (cfg.clk_rst_vif.rst_n) begin
      `uvm_info(`gfn, "\n\t ----| OTBN finished", UVM_MEDIUM)
    end else begin
      `uvm_info(`gfn, "\n\t ----| OTBN reset", UVM_MEDIUM)
    end
  endtask

  // How should we wait until OTBN has finished?
  //
  // Use polling if the interrupt is disabled or if the interrupt pin is already high. If enabled,
  // wait on the interrupt pin except occasionally (poll_despite_interrupts_pct). This exceptional
  // case is to spot any weirdness in updating the STATUS register when interrupts are enabled.
  protected function bit _pick_use_interrupt();
    uvm_status_e peek_status;
    uvm_reg_data_t peek_value;

    // We can just use the RAL prediction for the enable register (since it's only updated by TL
    // accesses).
    if (ral.intr_enable.done.get() == 0) begin
      return 1'b0;
    end

    // The interrupt is enabled, but maybe its state is already high. In that case, we can't use it
    // to spot whether we've finished.
    if (cfg.intr_vif.pins[0]) begin
      return 1'b0;
    end

    // The interrupt is enabled and not currently triggered. Use the interrupt except
    // poll_despite_interrupts_pct percent of the time.
    return $urandom_range(100) > cfg.poll_despite_interrupts_pct;
  endfunction

  // Monitor the bound-in loop controller interface to take action on loop warp events. Runs
  // forever, but is spawned by run_otbn(), which will kill it when the OTBN run completes or the
  // block is reset.
  protected task _run_loop_warps();
    logic [31:0] addr, old_iters, old_count;
    bit [31:0]   new_count, new_iters;

    forever begin
      // Run on the negative edge of the clock: we want to force a "_d" value, so should make sure
      // we get in after the DUT's logic that runs on posedge.
      @(negedge cfg.clk_rst_vif.clk);

      // If the loop stack is empty (so we don't have a current loop), there's nothing to do here.
      if (cfg.loop_vif.get_fullness() == StackEmpty)
        continue;

      // Get the current address and iteration counter.
      addr = cfg.loop_vif.insn_addr_i;
      old_iters = cfg.loop_vif.current_loop_d_iterations;

      // Convert from the "RTL view" of the iteration count (counting down to 1) to the "ISA view"
      // (counting up from zero).
      old_count = cfg.loop_vif.loop_iters_to_count(old_iters);

      // Do a DPI call to otbn_memutil to look up whether there is a loop warp that we should be
      // taking. This returns 1'b1 and fills in new_count if there is a warp that does something.
      if (!OtbnMemUtilGetLoopWarp(cfg.mem_util, addr, old_count, new_count))
        continue;

      // Convert this back to the "RTL view"
      new_iters = cfg.loop_vif.loop_count_to_iters(new_count);

      // Override the _d signal
      if (uvm_hdl_deposit({"tb.dut.u_otbn_core.u_otbn_controller.",
                           "u_otbn_loop_controller.current_loop_d.loop_iterations"},
                          new_iters) != 1) begin
        `dv_fatal("Failed to override loop_iterations for loop warp.")
      end
    end
  endtask

  virtual protected function string pick_elf_path();
    chandle helper;
    int     num_files;
    string  elf_path;

    // Check that cfg.otbn_elf_dir was set by the test
    `DV_CHECK_FATAL(cfg.otbn_elf_dir.len() > 0);

    // Pick an ELF file to use in the test. We have to do this via DPI (because you can't list a
    // directory in pure SystemVerilog). To do so, we have to construct a helper object, which will
    // look after memory allocation for the string holding the path.
    helper = OtbnTestHelperMake(cfg.otbn_elf_dir);
    `DV_CHECK_FATAL(helper != null)

    // Ask the helper how many files there are. If it returns zero, the directory name is bogus or
    // the directory is empty.
    num_files = OtbnTestHelperCountFilesInDir(helper);
    `DV_CHECK_FATAL(num_files > 0,
                    $sformatf("No regular files found in directory `%0s'.", cfg.otbn_elf_dir))

    // Pick a file, any file... Note that we pick an index on the SV side so that we use the right
    // random seed. Then we convert back to a filename with another DPI call. If the result is the
    // empty string, something went wrong.
    elf_path = OtbnTestHelperGetFilePath(helper, $urandom_range(num_files - 1));
    `DV_CHECK_FATAL(elf_path.len() > 0, "Bad index for ELF file")

    // Use sformat in a trivial way to take a copy of the string, so we can safely free helper (and
    // hence the old elf_path) afterwards.
    elf_path = $sformatf("%0s", elf_path);

    // Now that we've taken a copy of elf_path, we can safely free the test helper.
    OtbnTestHelperFree(helper);

    return elf_path;
  endfunction

  // Start running OTBN (forking off a run_otbn task) and return when it's still going.
  //
  // This works by guessing the number of cycles to run and then returning after that time. Of
  // course, the run_otbn task might actually finish in less time than that! If so, we run a second
  // time and guess a smaller value.
  //
  // This can be used to implement things like the reset test where we want to inject some sort of
  // error part-way into a run.
  task start_running_otbn(bit check_end_addr);
    bit timed_out = 1'b0;

    for (int i = 0; i < 10; i++) begin
      int cycle_counter;
      int unsigned max_wait_cycles;
      int unsigned wait_cycles;

      // Guess the number of cycles until error is injected. The maximum wait is "75% of the longest
      // we've seen the program run before terminating". This should avoid problems where we keep
      // picking times that occur after the program has finished.
      max_wait_cycles = ((longest_run_ > 0) ? longest_run_ : 1000) * 3 / 4;
      wait_cycles = $urandom_range(max_wait_cycles) + 1;
      fork: isolation_fork
      begin
        fork
          run_otbn(.check_end_addr(check_end_addr));
          begin
            wait (cfg.model_agent_cfg.vif.status == otbn_pkg::StatusBusyExecute);
            repeat (wait_cycles) begin
              @(cfg.clk_rst_vif.cbn);
              cycle_counter++;
            end
          end
        join_any

        // When we get here, we know that either the OTBN sequence finished or we timed out
        // and it's still going. We can see whether OTBN is still going by looking at the status
        // from the model (which is also in sync with the RTL). Because we wait on the negedge
        // when updating cycle_counter above, we know we've got the "new version" of the status at
        // this point.
        if (cfg.model_agent_cfg.vif.status == otbn_pkg::StatusBusyExecute) begin
          timed_out = 1'b1;
        end else begin
          timed_out = 1'b0;
          // The OTBN sequence finished so update wait_cycles. cycle_counter should always be less
          // than wait_cycles (because of how we calculate wait cycles).
          `DV_CHECK_FATAL(cycle_counter < wait_cycles);
          longest_run_ = cycle_counter;

          // Wait for the run_otbn thread to finish. This will usually be instant, but might take
          // a couple of cycles if we happen to have timed out exactly at the end of the run (when
          // the status has switched, but before run_otbn finishes)
          wait (!running_);

          // Kill the counter thread
          disable fork;
        end
      end
      join
      if (timed_out) break;
    end

    // If we get here and timed_out is false then something weird has happened: we've run the binary
    // 10 times and each run has taken less than 75% of the time of the previous run. This shouldn't
    // happen!
    `DV_CHECK_FATAL(timed_out, "Failed to pick a working time-out")
  endtask

  // Wait for (the one and only) interrupt to strike. Returns early on reset
  task wait_for_interrupt();
    if (cfg.clk_rst_vif.rst_n && !cfg.intr_vif.pins[0]) begin
      @(negedge cfg.clk_rst_vif.rst_n or posedge cfg.intr_vif.pins[0]);
    end
  endtask
endclass : otbn_base_vseq
