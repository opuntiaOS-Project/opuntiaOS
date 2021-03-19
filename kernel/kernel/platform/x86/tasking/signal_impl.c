#include <platform/generic/tasking/signal_impl.h>
#include <tasking/thread.h>

int signal_impl_prepare_stack(thread_t* thread, int signo, uint32_t old_sp, uint32_t magic)
{
    tf_push_to_stack(thread->tf, thread->tf->eflags);
    tf_push_to_stack(thread->tf, thread->tf->eip);
    tf_push_to_stack(thread->tf, thread->tf->eax);
    tf_push_to_stack(thread->tf, thread->tf->ebx);
    tf_push_to_stack(thread->tf, thread->tf->ecx);
    tf_push_to_stack(thread->tf, thread->tf->edx);
    tf_push_to_stack(thread->tf, old_sp);
    tf_push_to_stack(thread->tf, thread->tf->ebp);
    tf_push_to_stack(thread->tf, thread->tf->esi);
    tf_push_to_stack(thread->tf, thread->tf->edi);
    tf_push_to_stack(thread->tf, magic);
    tf_push_to_stack(thread->tf, (uint32_t)thread->signal_handlers[signo]);
    tf_push_to_stack(thread->tf, (uint32_t)signo);
    tf_push_to_stack(thread->tf, 0); /* fake return address */
    return 0;
}

int signal_impl_restore_stack(thread_t* thread, uint32_t* old_sp, uint32_t* magic)
{
    tf_move_stack_pointer(thread->tf, 12); /* cleaning 3 last pushes */
    *magic = tf_pop_to_stack(thread->tf);
    thread->tf->edi = tf_pop_to_stack(thread->tf);
    thread->tf->esi = tf_pop_to_stack(thread->tf);
    thread->tf->ebp = tf_pop_to_stack(thread->tf);
    *old_sp = tf_pop_to_stack(thread->tf);
    thread->tf->edx = tf_pop_to_stack(thread->tf);
    thread->tf->ecx = tf_pop_to_stack(thread->tf);
    thread->tf->ebx = tf_pop_to_stack(thread->tf);
    thread->tf->eax = tf_pop_to_stack(thread->tf);
    thread->tf->eip = tf_pop_to_stack(thread->tf);
    thread->tf->eflags = tf_pop_to_stack(thread->tf);
    return 0;
}