// See LICENSE file for copyright and license details.
static uint64_t ch_slice(const CapChannels ch, Cap *parent, Cap *child, uint64_t begin,
                  uint64_t end) {
        if (cap_is_child_ch(ch, cap_get(parent->next)))
                return -1;
        if (begin > end)
                return -1;
        CapChannels ch_child = cap_mk_channels(begin, end);
        if (cap_is_child_ch_ch(ch, ch_child))
                return -1;
        return cap_set(child, cap_serialize_channels(ch_child)) &&
               CapAppend(child, parent);
}

static uint64_t ch_split(const CapChannels ch, Cap *parent, Cap *child0, Cap *child1,
                  uint64_t mid) {
        if (cap_is_child_ch(ch, cap_get(parent->next)))
                return -1;
        if (child0 == child1)
                return -1;
        if (!(ch.begin < mid && mid <= ch.end))
                return -1;
        CapChannels ch_child0 = cap_mk_channels(ch.begin, mid);
        CapChannels ch_child1 = cap_mk_channels(mid, ch.end);
        return cap_set(child0, cap_serialize_channels(ch_child0)) &&
               cap_set(child1, cap_serialize_channels(ch_child1)) &&
               CapAppend(child0, parent) && CapAppend(child1, parent);
}

static uint64_t ch_instanciate(const CapChannels ch, Cap *parent, Cap *child0,
                        Cap *child1, uint64_t channel) {
        /* TODO: Check if channel is used */
        if (child0 == child1)
                return -1;
        CapReceiver recv_child0 = cap_mk_receiver(channel);
        CapSender send_child1 = cap_mk_sender(channel);
        if (!(ch.begin <= channel && channel <= ch.end))
                return -1;
        return cap_set(child0, cap_serialize_receiver(recv_child0)) &&
               cap_set(child1, cap_serialize_sender(send_child1)) &&
               CapAppend(child0, parent) && CapAppend(child1, parent);
}

uint64_t SyscallChannels(const CapChannels ch, Cap *cap, uint64_t a1,
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
                        /* Delete channels */
                        return CapDelete(cap);
                case SYSNR_REVOKE_CAP:
                        /* Revoke all channels  */
                        return CapRevoke(cap);
                case SYSNR_CH_SLICE:
                        /* Slice channels */
                        return ch_slice(ch, cap, curr_get_cap(a1), a2, a3);
                case SYSNR_CH_SPLIT:
                        /* Split channels */
                        return ch_split(ch, cap, curr_get_cap(a1),
                                        curr_get_cap(a2), a3);
                case SYSNR_CH_INSTANCIATE:
                        /* Instanciate channels, make receiver and sender */
                        return ch_instanciate(ch, cap, curr_get_cap(a1),
                                              curr_get_cap(a2), a3);
                default:
                        return -1;
        }
}
