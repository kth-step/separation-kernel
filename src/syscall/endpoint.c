// See LICENSE file for copyright and license details.
uint64_t sn_recv(uint64_t channel) {
        current->listen_channel = channel;
        while (current->listen_channel == channel) {
                asm volatile("");
        }
        /* We just wait to receive a message, sender does all the job */
        return current->args[0];
}
uint64_t sn_send(uint64_t channel, uint64_t caps_to_send, uint64_t msg[4]) {
        /* TODO */
        return -1;
}
uint64_t SyscallReceiver(const CapReceiver receiver, Cap *cap, uint64_t a1,
                         uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                         uint64_t a6, uint64_t a7) {
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
                case SYSNR_RC_RECEIVE:
                        /* Receive message */
                        return sn_recv(receiver.channel);
                default:
                        return -1;
        }
}

/*** SENDER HANDLER ***/
uint64_t SyscallSender(const CapSender sender, Cap *cap, uint64_t a1,
                       uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                       uint64_t a6, uint64_t a7) {
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
                case SYSNR_SN_SEND:
                        /* Send message */
                        return sn_send(sender.channel, a1,
                                       (uint64_t[4]){a2, a3, a4, a5});
                default:
                        return -1;
        }
}

