ENTRY(bf_activation_hook)
SECTIONS {
  . = 0x190200;
  .text : { *(.text*) }
  .rodata : { *(.rodata*) }
  .ARM.extab : { *(.ARM.extab*) }
  .ARM.exidx : { __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; }
  . = 0x188a60;
  .bss (NOLOAD) : { *(.bss*) *(COMMON) }
  /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
native_string_ctor = 0x14ea8;
native_string_assign = 0x14ef0;
native_string_dtor = 0x1540c;
native_app_ctor = 0x3984c;
native_app_execute = 0x18c54;
native_app_dtor = 0x398b4;
native_activation_resume = 0x27864;
native_activation_return = 0x27970;
native_writer_resume = 0x17fc4;
native_recent_resume = 0x188d4;
native_launch_cleanup = 0x18924;
__gxx_personality_v0 = 0x15280;
__cxa_begin_catch = 0x14e30;
__cxa_end_catch = 0x15508;
__cxa_end_cleanup = 0x1543c;
memset = 0x150e8;
memcpy = 0x15388;
/* R_ARM_NONE dependency markers only. Compact personalities are selected by
 * EHABI encodings; MainUI already imports both from its existing libgcc_s. */
__aeabi_unwind_cpp_pr0 = 0;
__aeabi_unwind_cpp_pr1 = 0;

native_startup_resume = 0x15a34;
