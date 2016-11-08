;Assembly language by csic.taobao.com
        THUMB
        REQUIRE8
        PRESERVE8

        AREA ||.text||, CODE, READONLY, ALIGN=2

fix_cs_params PROC
        LDR      r0,|L0.92|
        PUSH     {r4,lr}
        LDR      r1,[r0,#0]  
        CBZ      r1,|L0.26|
        LDR      r0,[r1,#0xa0]
        LDR      r1,[r1,#0x90]
        STR      r1,[r0,#0x2e0]
        MOVS     r1,#2
        BL       woal_init_priv
|L0.26|
        LDR      r0,|L0.96|
        LDR      r1,[r0,#0]  
        CMP      r1,#0
        BEQ      |L0.56|
        LDR      r0,[r1,#0xa0]
        LDR      r1,[r1,#0x90]
        STR      r1,[r0,#0x2e0]
        POP      {r4,lr}
        MOVS     r1,#2
        B.W      woal_init_priv
|L0.56|
        POP      {r4,pc}
        ENDP

init_monitor PROC
        PUSH     {r2-r4,lr}
        LDR      r0,|L0.100|
        LDRB     r0,[r0,#0]  
        CBZ      r0,|L0.86|
        ADR      r1,|L0.104|
        MOVS     r0,#0x80
        STRD     r0,r1,[sp,#0]
        MOVS     r3,#0
        MOVS     r2,#0xf
        MOV      r1,r3
        LDR      r0,|L0.112|
        BL       thread_create
|L0.86|
        POP      {r2-r4,lr}
        B        fix_cs_params
        ENDP

|L0.92|
        DCD      sta_wifi_dev
|L0.96|
        DCD      ap_wifi_dev
|L0.100|
        DCD      monitor_enable
|L0.104|
        DCB      "monitor",0
|L0.112|
        DCD      dev_monitor_task

        AREA ||.arm_vfe_header||, DATA, READONLY, NOALLOC, ALIGN=2

        DCD      0x00000000

        EXPORT fix_cs_params [CODE]
        EXPORT init_monitor [CODE]

        IMPORT ||Lib$$Request$$armlib|| [CODE,WEAK]
        IMPORT woal_init_priv [CODE]
        IMPORT sta_wifi_dev [DATA]
        IMPORT ap_wifi_dev [DATA]
        IMPORT thread_create [CODE]
        IMPORT monitor_enable [DATA]
        IMPORT dev_monitor_task [CODE]

        ATTR FILESCOPE
        ATTR SETVALUE Tag_ABI_PCS_wchar_t,2
        ATTR SETVALUE Tag_ABI_enum_size,1
        ATTR SETVALUE Tag_ABI_optimization_goals,3
        ATTR SETSTRING Tag_conformance,"2.06"
        ATTR SETVALUE AV,18,1

        ASSERT {ENDIAN} = "little"
        ASSERT {INTER} = {TRUE}
        ASSERT {ROPI} = {FALSE}
        ASSERT {RWPI} = {FALSE}
        ASSERT {IEEE_FULL} = {FALSE}
        ASSERT {IEEE_PART} = {FALSE}
        ASSERT {IEEE_JAVA} = {FALSE}
        END
