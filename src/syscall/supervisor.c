// See LICENSE file for copyright and license details.
static uint64_t sup_halt(Proc *supervisee) {
        /* Check if supervisee has halted or is to halt */
        if (supervisee->state == PROC_HALTED || supervisee->halt)
                return false;
        ProcHalt(supervisee);
        return true;
}

static uint64_t sup_is_halted(Proc *supervisee) {
        return supervisee->state == PROC_HALTED;
}

static uint64_t sup_resume(Proc *supervisee) {
        if (supervisee->state == PROC_HALTED) {
                supervisee->state = PROC_SUSPENDED;
                return true;
        }
        return false;
}

static uint64_t sup_reset(Proc *supervisee, Cap *pmp_cap) {
        CapData cd = cap_get(pmp_cap);
        if (cap_get_type(cd) != CAP_PMP_ENTRY)
                return -1;
        CapPmpEntry pe = cap_deserialize_pmp_entry(cd);
        if (supervisee->state != PROC_HALTED)
                return -1;
        /* Reset the process */
        ProcReset(supervisee->pid);
        return CapMove(&supervisee->cap_table[0], pmp_cap) &&
               ProcLoadPmp(supervisee, pe, pmp_cap, 0);
}

static uint64_t sup_read(Proc *supervisee, uint64_t cid) {
        return cap_get_arr(proc_get_cap(supervisee, cid), &current->args[1]);
}

static uint64_t sup_give(Proc *supervisee, uint64_t cid_dest, uint64_t cid_src) {
        return 0;
}

static uint64_t sup_take(Proc *supervisee, uint64_t cid_dest, uint64_t cid_src) {
        return 0;
}

uint64_t SyscallSupervisor(const CapSupervisor sup, Cap *cap, uint64_t a1,
                           uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                           uint64_t a6, uint64_t a7) {
        Proc *supervisee = &processes[sup.pid];
        switch (a7) {
                case SYSNR_READ_CAP:
                        /* Read cap */
                        return cap_get_arr(cap, &current->args[1]);
                case SYSNR_MOVE_CAP:
                        /* Move cap */
                        return CapMove(curr_get_cap(a1), cap);
                case SYSNR_DELETE_CAP:
                        /* Delete time slice */
                        return CapDelete(cap);
                case SYSNR_SP_IS_HALTED:
                        return sup_is_halted(supervisee);
                case SYSNR_SP_HALT:
                        /* Halt process */
                        return sup_halt(supervisee);
                case SYSNR_SP_RESUME:
                        /* Resume process */
                        return sup_resume(supervisee);
                case SYSNR_SP_RESET:
                        /* Reset process */
                        return sup_reset(supervisee, curr_get_cap(a1));
                case SYSNR_SP_READ:
                        /* Read cap */
                        return sup_read(supervisee, a1);
                case SYSNR_SP_GIVE:
                        /* Give cap */
                        return sup_give(supervisee, a1, a2);
                case SYSNR_SP_TAKE:
                        /* Take cap */
                        return sup_take(supervisee, a1, a2);
                default:
                        return -1;
        }
}
