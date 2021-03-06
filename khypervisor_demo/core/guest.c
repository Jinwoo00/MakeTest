#include <arch/arm/rtsm-config.h>
#include <guest.h>
#include <timer.h>
#include <interrupt.h>
#include <memory.h>
#include <vdev.h>
#include <hvmm_trace.h>
#include <smp.h>
#include <stdio.h>

#define NUM_GUEST_CONTEXTS        NUM_GUESTS_CPU0_STATIC

#define _valid_vmid(vmid) \
    (guest_first_vmid() <= vmid && guest_last_vmid() >= vmid)

static struct guest_struct guests[NUM_GUESTS_STATIC];
static int _current_guest_vmid[NUM_CPUS];// = {VMID_INVALID, VMID_INVALID};
static int _next_guest_vmid[NUM_CPUS];// = {VMID_INVALID, };
struct guest_struct *_current_guest[NUM_CPUS];
/* further switch request will be ignored if set */
static uint8_t _switch_locked[NUM_CPUS];

static hvmm_status_t guest_save(struct guest_struct *guest,
                        struct arch_regs *regs)
{
    /* guest_hw_save : save the current guest's context*/
    if (_guest_module.ops->save)
        return  _guest_module.ops->save(guest, regs);

    return HVMM_STATUS_UNKNOWN_ERROR;
}

static hvmm_status_t guest_restore(struct guest_struct *guest,
                        struct arch_regs *regs)
{
    /* guest_hw_restore : The next becomes the current */
    if (_guest_module.ops->restore)
        return  _guest_module.ops->restore(guest, regs);



     return HVMM_STATUS_UNKNOWN_ERROR;
}


static hvmm_status_t perform_switch(struct arch_regs *regs, vmid_t next_vmid)
{
    /* _curreng_guest_vmid -> next_vmid */

    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    struct guest_struct *guest = 0;
    uint32_t cpu = smp_processor_id();
    if (_current_guest_vmid[cpu] == next_vmid)
        return HVMM_STATUS_IGNORED; /* the same guest? */

    guest_save(&guests[_current_guest_vmid[cpu]], regs);
    memory_save();
    interrupt_save(_current_guest_vmid[cpu]);
    vdev_save(_current_guest_vmid[cpu]);

    /* The context of the next guest */
    guest = &guests[next_vmid];
    _current_guest[cpu] = guest;
    _current_guest_vmid[cpu] = next_vmid;

    /* guest_hw_dump */
    if (_guest_module.ops->dump)
        _guest_module.ops->dump(GUEST_VERBOSE_LEVEL_3, &guest->regs);

    vdev_restore(_current_guest_vmid[cpu]);

    interrupt_restore(_current_guest_vmid[cpu]);
    memory_restore(_current_guest_vmid[cpu]);
    guest_restore(guest, regs);

    return result;
}

hvmm_status_t guest_perform_switch(struct arch_regs *regs)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t cpu = smp_processor_id();

    if (_current_guest_vmid[cpu] == VMID_INVALID) {
        /*
         * If the scheduler is not already running, launch default
         * first guest. It occur in initial time.
         */
        printf("context: launching the first guest\n");

        result = perform_switch(0, _next_guest_vmid[cpu]);
        /* DOES NOT COME BACK HERE */
    } else if (_next_guest_vmid[cpu] != VMID_INVALID &&
                _current_guest_vmid[cpu] != _next_guest_vmid[cpu]) {
        printf("curr: %x\n", _current_guest_vmid[cpu]);
        printf("next: %x\n", _next_guest_vmid[cpu]);
        /* Only if not from Hyp */
        result = perform_switch(regs, _next_guest_vmid[cpu]);
        _next_guest_vmid[cpu] = VMID_INVALID;
    }

    _switch_locked[cpu] = 0;
    return result;
}

/* Switch to the first guest */
void guest_sched_start(void)
{
    struct guest_struct *guest = 0;
    uint32_t cpu = smp_processor_id();

    printf("[hyp] switch_to_initial_guest:\n");
    /* Select the first guest context to switch to. */
    _current_guest_vmid[cpu] = VMID_INVALID;
    if (cpu)
        guest = &guests[num_of_guest(cpu - 1) + 0];
    else
        guest = &guests[0];
    /* guest_hw_dump */
    if (_guest_module.ops->dump)
        _guest_module.ops->dump(GUEST_VERBOSE_LEVEL_0, &guest->regs);
    /* Context Switch with current context == none */

    guest_switchto(guest->vmid, 0);
    guest_perform_switch(&guest->regs);
}

vmid_t guest_first_vmid(void)
{
    uint32_t cpu = smp_processor_id();

    /* FIXME:Hardcoded for now */
#if _SMP_
    if (cpu)
        return 2;
    else
        return 0;
#endif
    return cpu;
}

vmid_t guest_last_vmid(void)
{
    uint32_t cpu = 1;// smp_processor_id();

    /* FIXME:Hardcoded for now */
#if _SMP_
    if (cpu)
        return 3;
    else
        return 1;
#endif
    return cpu;
}

vmid_t guest_next_vmid(vmid_t ofvmid)
{
    vmid_t next = VMID_INVALID;
#if 0
#ifdef _SMP_
    uint32_t cpu = smp_processor_id();

    if (cpu)
        return 2;
    else
        return 0;
#endif
#endif

    /* FIXME:Hardcoded */
    if (ofvmid == VMID_INVALID)
        next = guest_first_vmid();
    else if (ofvmid < guest_last_vmid()) {
        /* FIXME:Hardcoded */
        next = ofvmid + 1;
    }
    return next;
}

vmid_t guest_current_vmid(void)
{
    uint32_t cpu = smp_processor_id();
    return _current_guest_vmid[cpu];
}

vmid_t guest_waiting_vmid(void)
{
    uint32_t cpu = smp_processor_id();
    return _next_guest_vmid[cpu];
}

void guest_dump_regs(struct arch_regs *regs)
{
    /* guest_hw_dump */
    _guest_module.ops->dump(GUEST_VERBOSE_ALL, regs);
}

hvmm_status_t guest_switchto(vmid_t vmid, uint8_t locked)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t cpu = smp_processor_id();

    /* valid and not current vmid, switch */
    if (_switch_locked[cpu] == 0) {
        _next_guest_vmid[cpu] = vmid;
        result = HVMM_STATUS_SUCCESS;
        printf("switching to vmid: %x\n", (uint32_t)vmid);
    } else
        printf("context: next vmid locked to %d\n", _next_guest_vmid[cpu]);

    if (locked)
        _switch_locked[cpu] = locked;

    return result;
}

static int manually_next_vmid;
vmid_t selected_manually_next_vmid;
void set_manually_select_vmid(vmid_t vmid)
{
    manually_next_vmid = 1;
    selected_manually_next_vmid = vmid;
}
void clean_manually_select_vmid(void){
    manually_next_vmid = 0;
}

vmid_t sched_policy_determ_next(void)
{
    printf("scheduler call\n");
#if 1
    if (manually_next_vmid)
        return selected_manually_next_vmid;

    vmid_t next = guest_next_vmid(guest_current_vmid());
    printf("current :%d next VMID = %d\n", guest_current_vmid(), next);

    /* FIXME:Hardcoded */
    if (next == VMID_INVALID)
        next = guest_first_vmid();

    return next;
#endif
//    return guest_first_vmid();
}

void guest_schedule(void *pdata)
{
    struct arch_regs *regs = pdata;
    //uint32_t cpu = smp_processor_id();
    /* guest_hw_dump */
    if (_guest_module.ops->dump)
        _guest_module.ops->dump(GUEST_VERBOSE_LEVEL_3, regs);
    /*
     * Note: As of guest_switchto() and guest_perform_switch()
     * are available, no need to test if trapped from Hyp mode.
     * guest_perform_switch() takes care of it
     */

    /* Switch request, actually performed at trap exit */
    guest_switchto(sched_policy_determ_next(), 0);

}

hvmm_status_t guest_init()
{
    struct timer_val timer;
    hvmm_status_t result = HVMM_STATUS_SUCCESS;
    struct guest_struct *guest;
    struct arch_regs *regs = 0;
    int i;
    int guest_count;
    int start_vmid = 0;
    uint32_t cpu = smp_processor_id();
    printf("[hyp] init_guests: enter\n");
    /* Initializes guests */
    guest_count = num_of_guest(cpu);


    if (cpu)
        start_vmid = num_of_guest(cpu - 1);
    else
        start_vmid = 0;

    guest_count += start_vmid;

    for (i = start_vmid; i < guest_count; i++) {
        /* Guest i @guest_bin_start */
        guest = &guests[i];
        regs = &guest->regs;
        guest->vmid = i;
        /* guest_hw_init */
        if (_guest_module.ops->init)
            _guest_module.ops->init(guest, regs);
    }

    printf("[hyp] init_guests: return\n");

    /* 100Mhz -> 1 count == 10ns at RTSM_VE_CA15, fast model*/
    timer.interval_us = GUEST_SCHED_TICK;
    timer.callback = &guest_schedule;

    result = timer_set(&timer, HOST_TIMER);

    if (result != HVMM_STATUS_SUCCESS)
        printf("[%s] timer startup failed...\n", __func__);

    return result;
}

#if 0
struct guest_struct get_guest(uint32_t guest_num)
{
   return guests[guest_num];
}
#endif

void guest_copy(struct guest_struct *dst, vmid_t vmid_src)
{
    _guest_module.ops->move(dst, &(guests[vmid_src]));
}

void reboot_guest(vmid_t vmid, uint32_t pc,
        struct arch_regs **regs)
{
    _guest_module.ops->init(&guests[vmid], &(guests[vmid].regs));
    guests[vmid].regs.pc = pc;
    guests[vmid].regs.gpr[10] = 1;
    if (regs != 0)
        _guest_module.ops->restore(&guests[vmid], *regs);
}

