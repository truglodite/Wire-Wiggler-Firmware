#ifdef RAMPS_V_1_4_MEGA2560

    // https://kingshobbiesbrasil.wixsite.com/p3dp/f-rum/projetos-dicas/esquema-eletronico-da-placa-anet-a8
    #define aziStep      A0  // X step
    #define aziDir       A1  // X direction
    #define aziEN        38  // X enable
    #define eleStep      A6  // Y step
    #define eleDir       A7  // Y direction
    #define eleEN        A2  // Y enable
    #define polStep      46  // Z step
    #define polDir       48  // Z direction
    #define polEN        A8  // Z enable
    #define tunStep      26  // E0 step
    #define tunDir       28  // E0 direction
    #define tunEN        24  // E0 enable
    #define auxStep      36  // E1 step
    #define auxDir       34  // E1 direction
    #define auxEN        30  // E1 enable
    // Code uses internal pullups
    #define aziMinStop   3   // X- endstop
    #define aziMaxStop   2   // X+ endstop
    #define eleMinStop   14  // Y- endstop
    #define eleMaxStop   15  // Y+ endstop
    #define polMinStop   18  // Z- endstop
    #define polMaxStop   19  // Z+ endstop

#endif // RAMPS_V_1_4_MEGA2560