#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_aarch64_elf64_le_vec)

{ "aarch64-*-elf",
&aarch64_elf64_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_aarch64_elf64_be_vec)

{ "aarch64_be-*-elf",
&aarch64_elf64_be_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_aarch64_elf64_le_vec)

{ "aarch64-*-linux*",
&aarch64_elf64_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_aarch64_elf64_be_vec)

{ "aarch64_be-*-linux*",
&aarch64_elf64_be_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_alpha_elf64_fbsd_vec)

{ "alpha*-*-freebsd*", NULL },{ "alpha*-*-kfreebsd*-gnu",
&alpha_elf64_fbsd_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_alpha_elf64_vec)

{ "alpha*-*-netbsd*", NULL },{ "alpha*-*-openbsd*",
&alpha_elf64_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_alpha_ecoff_le_vec)

{ "alpha*-*-netware*",
&alpha_ecoff_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_alpha_ecoff_le_vec)

{ "alpha*-*-linux*ecoff*",
&alpha_ecoff_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_alpha_elf64_vec)

{ "alpha*-*-linux-*", NULL },{ "alpha*-*-elf*",
&alpha_elf64_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_alpha_vms_vec)

{ "alpha*-*-*vms*",
&alpha_vms_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_alpha_ecoff_le_vec)

{ "alpha*-*-*",
&alpha_ecoff_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_ia64_elf64_le_vec)

{ "ia64*-*-freebsd*", NULL },{ "ia64*-*-netbsd*", NULL },{ "ia64*-*-linux-*", NULL },{ "ia64*-*-elf*", NULL },{ "ia64*-*-kfreebsd*-gnu",
&ia64_elf64_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_ia64_elf32_hpux_be_vec)

{ "ia64*-*-hpux*",
&ia64_elf32_hpux_be_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_ia64_elf64_vms_vec)

{ "ia64*-*-*vms*",
&ia64_elf64_vms_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf64_fbsd_vec)

{ "sparc64-*-freebsd*", NULL },{ "sparc64-*-kfreebsd*-gnu",
&sparc_elf64_fbsd_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf64_vec)

{ "sparc64-*-netbsd*", NULL },{ "sparc64-*-openbsd*",
&sparc_elf64_vec },
#endif



    
#endif /* BFD64 */

#if !defined (SELECT_VECS) || defined (HAVE_am33_elf32_linux_vec)

{ "am34-*-linux*", NULL },{ "am33_2.0-*-linux*",
&am33_elf32_linux_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_arc_elf32_le_vec)

{ "arc-*-elf*",
&arc_elf32_le_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_arm_elf32_nacl_le_vec)

{ "arm-*-nacl*",
&arm_elf32_nacl_le_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_arm_elf32_nacl_be_vec)

{ "armeb-*-nacl*",
&arm_elf32_nacl_be_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_arm_elf32_be_vec)

{ "armeb-*-netbsdelf*",
&arm_elf32_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_arm_elf32_le_vec)

{ "arm-*-netbsdelf*",
&arm_elf32_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_arm_aout_nbsd_vec)

{ "arm-*-netbsd*", NULL },{ "arm-*-openbsd*",
&arm_aout_nbsd_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_arm_elf32_le_vec)

{ "arm-*-nto*", NULL },{ "nto*arm*",
&arm_elf32_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_arm_aout_riscix_vec)

{ "arm-*-riscix*",
&arm_aout_riscix_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_arm_pe_epoc_le_vec)

{ "arm-epoc-pe*",
&arm_pe_epoc_le_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_arm_pe_wince_le_vec)

{ "arm-wince-pe", NULL },{ "arm-*-wince", NULL },{ "arm*-*-mingw32ce*", NULL },{ "arm*-*-cegcc*",
&arm_pe_wince_le_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_arm_pe_le_vec)

{ "arm-*-pe*",
&arm_pe_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_arm_aout_le_vec)

{ "arm-*-aout", NULL },{ "armel-*-aout",
&arm_aout_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_arm_aout_be_vec)

{ "armeb-*-aout",
&arm_aout_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_arm_coff_le_vec)

{ "arm-*-coff",
&arm_coff_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_arm_elf32_le_vec)

{ "arm-*-rtems*",
&arm_elf32_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_arm_elf32_be_vec)

{ "armeb-*-elf", NULL },{ "arm*b-*-freebsd*", NULL },{ "arm*b-*-linux-*", NULL },{ "armeb-*-eabi*",
&arm_elf32_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_arm_elf32_le_vec)

{ "arm-*-kaos*",
&arm_elf32_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_arm_elf32_le_vec)

{ "arm-*-elf", NULL },{ "arm*-*-freebsd*", NULL },{ "arm*-*-linux-*", NULL },{ "arm*-*-conix*", NULL },
{ "arm*-*-uclinux*", NULL },{ "arm-*-kfreebsd*-gnu", NULL },
{ "arm*-*-eabi*",
&arm_elf32_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_arm_elf32_vxworks_le_vec)

{ "arm*-*-vxworks", NULL },{ "arm*-*-windiss",
&arm_elf32_vxworks_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_arm_elf32_symbian_le_vec)

{ "arm*-*-symbianelf*",
&arm_elf32_symbian_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_arm_elf32_le_vec)

{ "arm9e-*-elf",
&arm_elf32_le_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_avr_elf32_vec)

{ "avr-*-*",
&avr_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_bfin_elf32_vec)

{ "bfin-*-*",
&bfin_elf32_vec },
#endif



    

#if !defined (SELECT_VECS) || defined (HAVE_tic30_aout_vec)

{ "c30-*-*aout*", NULL },{ "tic30-*-*aout*",
&tic30_aout_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_tic30_coff_vec)

{ "c30-*-*coff*", NULL },{ "tic30-*-*coff*",
&tic30_coff_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_tic4x_coff1_vec)

{ "c4x-*-*coff*", NULL },{ "tic4x-*-*coff*", NULL },{ "tic4x-*-rtems*",
&tic4x_coff1_vec },
#endif



    

#if !defined (SELECT_VECS) || defined (HAVE_tic54x_coff1_vec)

{ "c54x*-*-*coff*", NULL },{ "tic54x-*-*coff*",
&tic54x_coff1_vec },
#endif



    

#if !defined (SELECT_VECS) || defined (HAVE_cr16_elf32_vec)

{ "cr16-*-elf*", NULL },{ "cr16*-*-uclinux*",
&cr16_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_cr16c_elf32_vec)

{ "cr16c-*-elf*",
&cr16c_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_cris_aout_vec)

{ "cris-*-*", NULL },{ "crisv32-*-*",
&cris_aout_vec },
#endif



    

#if !defined (SELECT_VECS) || defined (HAVE_crx_elf32_vec)

{ "crx-*-elf*",
&crx_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_d10v_elf32_vec)

{ "d10v-*-*",
&d10v_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_dlx_elf32_be_vec)

{ "dlx-*-elf*",
&dlx_elf32_be_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_d30v_elf32_vec)

{ "d30v-*-*",
&d30v_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_epiphany_elf32_vec)

{ "epiphany-*-elf",
&epiphany_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_m68k_elf32_vec)

{ "fido-*-elf*",
&m68k_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_fr30_elf32_vec)

{ "fr30-*-elf",
&fr30_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_frv_elf32_vec)

{ "frv-*-elf",
&frv_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_frv_elf32_fdpic_vec)

{ "frv-*-*linux*",
&frv_elf32_fdpic_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_moxie_elf32_be_vec)

{ "moxie-*-elf", NULL },{ "moxie-*-rtems*", NULL },{ "moxie-*-uclinux",
&moxie_elf32_be_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_moxie_elf32_le_vec)

{ "moxie-*-moxiebox*",
&moxie_elf32_le_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_h8300_coff_vec)

{ "h8300*-*-rtemscoff*",
&h8300_coff_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_h8300_elf32_vec)

{ "h8300*-*-elf", NULL },{ "h8300*-*-rtems*",
&h8300_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_h8300_coff_vec)

{ "h8300*-*-*",
&h8300_coff_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_h8500_coff_vec)

{ "h8500-*-*",
&h8500_coff_vec },
#endif


    

#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_hppa_elf64_linux_vec)

{ "hppa*64*-*-linux-*",
&hppa_elf64_linux_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_hppa_elf64_vec)

{ "hppa*64*-*-hpux11*",
&hppa_elf64_vec },
#endif




    
#endif

#if !defined (SELECT_VECS) || defined (HAVE_hppa_elf32_linux_vec)

{ "hppa*-*-linux-*",
&hppa_elf32_linux_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_hppa_elf32_nbsd_vec)

{ "hppa*-*-netbsd*",
&hppa_elf32_nbsd_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_hppa_elf32_vec)

{ "hppa*-*-*elf*", NULL },{ "hppa*-*-lites*", NULL },{ "hppa*-*-sysv4*", NULL },{ "hppa*-*-openbsd*",
&hppa_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_hppa_som_vec)

{ "hppa*-*-bsd*",
&hppa_som_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_hppa_som_vec)

{ "hppa*-*-hpux*", NULL },{ "hppa*-*-hiux*", NULL },{ "hppa*-*-mpeix*",
&hppa_som_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_hppa_som_vec)

{ "hppa*-*-osf*",
&hppa_som_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_i370_elf32_vec)

{ "i370-*-*",
&i370_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_coff_vec)

{ "i[3-7]86-*-sco3.2v5*coff",
&i386_coff_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-sysv4*", NULL },{ "i[3-7]86-*-unixware*", NULL },
{ "i[3-7]86-*-elf", NULL },{ "i[3-7]86-*-sco3.2v5*", NULL },
{ "i[3-7]86-*-dgux*", NULL },{ "i[3-7]86-*-sysv5*",
&i386_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_sol2_vec)

{ "i[3-7]86-*-solaris2*",
&i386_elf32_sol2_vec },
#endif




    
#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_sol2_vec)

{ "x86_64-*-solaris2*",
&i386_elf32_sol2_vec },
#endif



    
#endif
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-kaos*",
&i386_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-nto*",
&i386_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-aros*",
&i386_elf32_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-chorus*",
&i386_elf32_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-dicos*",
&i386_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_coff_go32_vec)

{ "*-*-msdosdjgpp*", NULL },{ "*-*-go32*",
&i386_coff_go32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_coff_vec)

{ "i[3-7]86-*-sysv*", NULL },{ "i[3-7]86-*-isc*", NULL },{ "i[3-7]86-*-sco*", NULL },{ "i[3-7]86-*-coff", NULL },
{ "i[3-7]86-*-aix*",
&i386_coff_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-rtems*",
&i386_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_mach_o_vec)

{ "i[3-7]86-*-darwin*", NULL },{ "i[3-7]86-*-macos10*", NULL },{ "i[3-7]86-*-rhapsody*",
&i386_mach_o_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_i386_aout_dynix_vec)

{ "i[3-7]86-sequent-bsd*",
&i386_aout_dynix_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_aout_bsd_vec)

{ "i[3-7]86-*-bsd*",
&i386_aout_bsd_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-dragonfly*",
&i386_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_aout_fbsd_vec)

{ "i[3-7]86-*-freebsdaout*", NULL },{ "i[3-7]86-*-freebsd[12].*", NULL },
{ "i[3-7]86-*-freebsd[12]",
&i386_aout_fbsd_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_fbsd_vec)

{ "i[3-7]86-*-freebsd*", NULL },{ "i[3-7]86-*-kfreebsd*-gnu",
&i386_elf32_fbsd_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-netbsdelf*", NULL },{ "i[3-7]86-*-netbsd*-gnu*", NULL },{ "i[3-7]86-*-knetbsd*-gnu",
&i386_elf32_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_i386_pe_vec)

{ "i[3-7]86-*-netbsdpe*",
&i386_pe_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_aout_nbsd_vec)

{ "i[3-7]86-*-netbsdaout*", NULL },{ "i[3-7]86-*-netbsd*", NULL },
{ "i[3-7]86-*-openbsd[0-2].*", NULL },{ "i[3-7]86-*-openbsd3.[0-3]",
&i386_aout_nbsd_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-openbsd*",
&i386_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-netware*",
&i386_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_aout_linux_vec)

{ "i[3-7]86-*-linux*aout*",
&i386_aout_linux_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-linux-*",
&i386_elf32_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_nacl_vec)

{ "i[3-7]86-*-nacl*",
&i386_elf32_nacl_vec },
#endif




    
#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_x86_64_mach_o_vec)

{ "x86_64-*-darwin*",
&x86_64_mach_o_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_x86_64_elf64_vec)

{ "x86_64-*-dicos*",
&x86_64_elf64_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_x86_64_elf64_vec)

{ "x86_64-*-elf*",
&x86_64_elf64_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_x86_64_elf64_vec)

{ "x86_64-*-dragonfly*",
&x86_64_elf64_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_x86_64_elf64_fbsd_vec)

{ "x86_64-*-freebsd*", NULL },{ "x86_64-*-kfreebsd*-gnu",
&x86_64_elf64_fbsd_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_x86_64_elf64_vec)

{ "x86_64-*-netbsd*", NULL },{ "x86_64-*-openbsd*",
&x86_64_elf64_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_x86_64_elf64_vec)

{ "x86_64-*-linux-*",
&x86_64_elf64_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_x86_64_elf32_nacl_vec)

{ "x86_64-*-nacl*",
&x86_64_elf32_nacl_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_x86_64_pe_vec)

{ "x86_64-*-mingw*", NULL },{ "x86_64-*-pe", NULL },{ "x86_64-*-pep", NULL },{ "x86_64-*-cygwin",
&x86_64_pe_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_x86_64_elf64_vec)

{ "x86_64-*-rdos*",
&x86_64_elf64_vec },
#endif


    
#endif
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-lynxos*",
&i386_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-gnu*",
&i386_elf32_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_i386_aout_mach3_vec)

{ "i[3-7]86-*-mach*", NULL },{ "i[3-7]86-*-osf1mk*",
&i386_aout_mach3_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_i386_aout_os9k_vec)

{ "i[3-7]86-*-os9k",
&i386_aout_os9k_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_i386_aout_vec)

{ "i[3-7]86-*-msdos*",
&i386_aout_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-moss*",
&i386_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_pe_vec)

{ "i[3-7]86-*-beospe*",
&i386_pe_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-beoself*", NULL },{ "i[3-7]86-*-beos*",
&i386_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_pei_vec)

{ "i[3-7]86-*-interix*",
&i386_pei_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-rdos*",
&i386_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_pe_vec)

{ "i[3-7]86-*-mingw32*", NULL },{ "i[3-7]86-*-cygwin*", NULL },{ "i[3-7]86-*-winnt", NULL },{ "i[3-7]86-*-pe",
&i386_pe_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_i386_coff_vec)

{ "i[3-7]86-none-*",
&i386_coff_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_i386_aout_vec)

{ "i[3-7]86-*-aout*", NULL },{ "i[3-7]86*-*-vsta*",
&i386_aout_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vxworks_vec)

{ "i[3-7]86-*-vxworks*",
&i386_elf32_vxworks_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i386_elf32_vec)

{ "i[3-7]86-*-chaos",
&i386_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_i860_coff_vec)

{ "i860-*-mach3*", NULL },{ "i860-*-osf1*", NULL },{ "i860-*-coff*",
&i860_coff_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_i860_elf32_le_vec)

{ "i860-stardent-sysv4*", NULL },{ "i860-stardent-elf*",
&i860_elf32_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_i860_elf32_vec)

{ "i860-*-sysv4*", NULL },{ "i860-*-elf*",
&i860_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_bout_le_vec)

{ "i960-*-vxworks4*", NULL },{ "i960-*-vxworks5.0",
&bout_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_icoff_le_vec)

{ "i960-*-vxworks5.*", NULL },{ "i960-*-coff*", NULL },{ "i960-*-sysv*",
&icoff_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_bout_le_vec)

{ "i960-*-vxworks*", NULL },{ "i960-*-aout*", NULL },{ "i960-*-bout*", NULL },{ "i960-*-nindy*",
&bout_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_i960_elf32_vec)

{ "i960-*-elf*",
&i960_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_ip2k_elf32_vec)

{ "ip2k-*-elf",
&ip2k_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_iq2000_elf32_vec)

{ "iq2000-*-elf",
&iq2000_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_lm32_elf32_vec)

{ "lm32-*-elf", NULL },{ "lm32-*-rtems*",
&lm32_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_lm32_elf32_fdpic_vec)

{ "lm32-*-*linux*",
&lm32_elf32_fdpic_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_m32c_elf32_vec)

{ "m32c-*-elf", NULL },{ "m32c-*-rtems*",
&m32c_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_m32r_elf32_linux_le_vec)

{ "m32r*le-*-linux*",
&m32r_elf32_linux_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m32r_elf32_linux_vec)

{ "m32r*-*-linux*",
&m32r_elf32_linux_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m32r_elf32_le_vec)

{ "m32r*le-*-*",
&m32r_elf32_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m32r_elf32_vec)

{ "m32r-*-*",
&m32r_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_m68hc11_elf32_vec)

{ "m68hc11-*-*", NULL },{ "m6811-*-*",
&m68hc11_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m68hc12_elf32_vec)

{ "m68hc12-*-*", NULL },{ "m6812-*-*",
&m68hc12_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_m68k_coff_sysv_vec)

{ "m68*-motorola-sysv*",
&m68k_coff_sysv_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_aout_hp300bsd_vec)

{ "m68*-hp-bsd*",
&m68k_aout_hp300bsd_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_aout0_be_vec)

{ "m68*-*-aout*",
&aout0_be_vec },
#endif






    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_elf32_vec)

{ "m68*-*-elf*", NULL },{ "m68*-*-sysv4*", NULL },{ "m68*-*-uclinux*",
&m68k_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_elf32_vec)

{ "m68*-*-rtems*",
&m68k_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_coff_vec)

{ "m68*-*-coff*", NULL },{ "m68*-*-sysv*",
&m68k_coff_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_aout_hp300hpux_vec)

{ "m68*-*-hpux*",
&m68k_aout_hp300hpux_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_aout_linux_vec)

{ "m68*-*-linux*aout*",
&m68k_aout_linux_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_elf32_vec)

{ "m68*-*-linux-*",
&m68k_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_elf32_vec)

{ "m68*-*-gnu*",
&m68k_elf32_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_aout_4knbsd_vec)

{ "m68*-hp*-netbsd*",
&m68k_aout_4knbsd_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_elf32_vec)

{ "m68*-*-netbsdelf*",
&m68k_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_aout_nbsd_vec)

{ "m68*-*-netbsdaout*", NULL },{ "m68*-*-netbsd*",
&m68k_aout_nbsd_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_aout_nbsd_vec)

{ "m68*-*-openbsd*",
&m68k_aout_nbsd_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_aout_sunos_be_vec)

{ "m68*-*-sunos*", NULL },{ "m68*-*-os68k*", NULL },{ "m68*-*-vxworks*", NULL },{ "m68*-netx-*", NULL },
{ "m68*-*-bsd*", NULL },{ "m68*-*-vsta*",
&sparc_aout_sunos_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_aout_sunos_be_vec)

{ "m68*-ericsson-*",
&sparc_aout_sunos_be_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_elf32_vec)

{ "m68*-cbm-*",
&m68k_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m68k_elf32_vec)

{ "m68*-*-psos*",
&m68k_elf32_vec },
#endif



    

#if !defined (SELECT_VECS) || defined (HAVE_m88k_elf32_vec)

{ "m88*-harris-cxux*", NULL },{ "m88*-*-dgux*", NULL },{ "m88*-*-sysv4*",
&m88k_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m88k_aout_mach3_vec)

{ "m88*-*-mach3*",
&m88k_aout_mach3_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_m88k_aout_obsd_vec)

{ "m88*-*-openbsd*",
&m88k_aout_obsd_vec },
#endif


   
#if !defined (SELECT_VECS) || defined (HAVE_m88k_coff_bcs_vec)

{ "m88*-*-*",
&m88k_coff_bcs_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_mcore_elf32_be_vec)

{ "mcore-*-elf",
&mcore_elf32_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mcore_pe_be_vec)

{ "mcore-*-pe",
&mcore_pe_be_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_mep_elf32_vec)

{ "mep-*-elf",
&mep_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_metag_elf32_vec)

{ "metag-*-*",
&metag_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_microblaze_elf32_le_vec)

{ "microblazeel*-*",
&microblaze_elf32_le_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_microblaze_elf32_vec)

{ "microblaze*-*",
&microblaze_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_mips_ecoff_be_vec)

{ "mips*-big-*",
&mips_ecoff_be_vec },
#endif


    
#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_trad_le_vec)

{ "mips*el-*-netbsd*",
&mips_elf32_trad_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_trad_be_vec)

{ "mips*-*-netbsd*",
&mips_elf32_trad_be_vec },
#endif


    
#endif
#if !defined (SELECT_VECS) || defined (HAVE_mips_ecoff_le_vec)

{ "mips*-dec-*", NULL },{ "mips*el-*-ecoff*",
&mips_ecoff_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_ecoff_be_vec)

{ "mips*-*-ecoff*",
&mips_ecoff_be_vec },
#endif


    
#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_n_be_vec)

{ "mips*-*-irix6*",
&mips_elf32_n_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_n_le_vec)

{ "mips64*-ps2-elf*",
&mips_elf32_n_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_le_vec)

{ "mips*-ps2-elf*",
&mips_elf32_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_be_vec)

{ "mips*-*-irix5*",
&mips_elf32_be_vec },
#endif


    
#endif
#if !defined (SELECT_VECS) || defined (HAVE_mips_ecoff_be_vec)

{ "mips*-sgi-*", NULL },{ "mips*-*-bsd*",
&mips_ecoff_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_ecoff_bele_vec)

{ "mips*-*-lnews*",
&mips_ecoff_bele_vec },
#endif


    
#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_trad_be_vec)

{ "mips*-*-sysv4*",
&mips_elf32_trad_be_vec },
#endif


    
#endif
#if !defined (SELECT_VECS) || defined (HAVE_mips_ecoff_be_vec)

{ "mips*-*-sysv*", NULL },{ "mips*-*-riscos*",
&mips_ecoff_be_vec },
#endif


    
#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_vxworks_le_vec)

{ "mips*el-*-vxworks*",
&mips_elf32_vxworks_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_vxworks_be_vec)

{ "mips*-*-vxworks*",
&mips_elf32_vxworks_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_trad_le_vec)

{ "mips*el-sde-elf*",
&mips_elf32_trad_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_trad_be_vec)

{ "mips*-sde-elf*", NULL },{ "mips*-mti-elf*", NULL },{ "mips*-img-elf*",
&mips_elf32_trad_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_le_vec)

{ "mips*el-*-elf*", NULL },{ "mips*el-*-vxworks*", NULL },{ "mips*-*-chorus*",
&mips_elf32_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_be_vec)

{ "mips*-*-elf*", NULL },{ "mips*-*-rtems*", NULL },{ "mips*-*-vxworks", NULL },{ "mips*-*-windiss",
&mips_elf32_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_be_vec)

{ "mips*-*-none",
&mips_elf32_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf64_trad_be_vec)

{ "mips64*-*-openbsd*",
&mips_elf64_trad_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_le_vec)

{ "mips*el-*-openbsd*",
&mips_elf32_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_be_vec)

{ "mips*-*-openbsd*",
&mips_elf32_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_ntrad_le_vec)

{ "mips64*el-*-linux*",
&mips_elf32_ntrad_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_ntrad_be_vec)

{ "mips64*-*-linux*",
&mips_elf32_ntrad_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_trad_le_vec)

{ "mips*el-*-linux*",
&mips_elf32_trad_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_trad_be_vec)

{ "mips*-*-linux*",
&mips_elf32_trad_be_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_ntradfbsd_le_vec)

{ "mips64*el-*-freebsd*", NULL },{ "mips64*el-*-kfreebsd*-gnu",
&mips_elf32_ntradfbsd_le_vec },
#endif




    

#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_ntradfbsd_be_vec)

{ "mips64*-*-freebsd*", NULL },{ "mips64*-*-kfreebsd*-gnu",
&mips_elf32_ntradfbsd_be_vec },
#endif




    

#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_tradfbsd_le_vec)

{ "mips*el-*-freebsd*", NULL },{ "mips*el-*-kfreebsd*-gnu",
&mips_elf32_tradfbsd_le_vec },
#endif




    

#if !defined (SELECT_VECS) || defined (HAVE_mips_elf32_tradfbsd_be_vec)

{ "mips*-*-freebsd*", NULL },{ "mips*-*-kfreebsd*-gnu",
&mips_elf32_tradfbsd_be_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_mmix_elf64_vec)

{ "mmix-*-*",
&mmix_elf64_vec },
#endif



    
#endif
#if !defined (SELECT_VECS) || defined (HAVE_mn10200_elf32_vec)

{ "mn10200-*-*",
&mn10200_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_mn10300_elf32_vec)

{ "mn10300-*-*",
&mn10300_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_mt_elf32_vec)

{ "mt-*-elf",
&mt_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_msp430_elf32_vec)

{ "msp430-*-*",
&msp430_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_nds32_elf32_linux_le_vec)

{ "nds32*le-*-linux*",
&nds32_elf32_linux_le_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_nds32_elf32_linux_be_vec)

{ "nds32*be-*-linux*",
&nds32_elf32_linux_be_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_nds32_elf32_le_vec)

{ "nds32*le-*-*",
&nds32_elf32_le_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_nds32_elf32_be_vec)

{ "nds32*be-*-*",
&nds32_elf32_be_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_ns32k_aout_pc532mach_vec)

{ "ns32k-pc532-mach*", NULL },{ "ns32k-pc532-ux*",
&ns32k_aout_pc532mach_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_ns32k_aout_pc532nbsd_vec)

{ "ns32k-*-netbsd*", NULL },{ "ns32k-*-lites*", NULL },{ "ns32k-*-openbsd*",
&ns32k_aout_pc532nbsd_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_nios2_elf32_be_vec)

{ "nios2eb-*-*",
&nios2_elf32_be_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_nios2_elf32_le_vec)

{ "nios2el-*-*",
&nios2_elf32_le_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_nios2_elf32_le_vec)

{ "nios2-*-*",
&nios2_elf32_le_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_or1k_elf32_vec)

{ "or1k-*-elf", NULL },{ "or1k-*-linux*", NULL },{ "or1k-*-rtems*",
&or1k_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_or1k_elf32_vec)

{ "or1knd-*-elf", NULL },{ "or1knd-*-linux*", NULL },{ "or1knd-*-rtems*",
&or1k_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_pdp11_aout_vec)

{ "pdp11-*-*",
&pdp11_aout_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_pj_elf32_vec)

{ "pj-*-*",
&pj_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_pj_elf32_le_vec)

{ "pjl-*-*",
&pj_elf32_le_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_rs6000_xcoff_vec)

{ "powerpc-*-aix5.[01]", NULL },{ "rs6000-*-aix5.[01]",
&rs6000_xcoff_vec },
#endif



    
#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_rs6000_xcoff64_aix_vec)

{ "powerpc64-*-aix5.[01]", NULL },{ "rs6000-*-aix5.[01]",
&rs6000_xcoff64_aix_vec },
#endif



    
#endif

#if !defined (SELECT_VECS) || defined (HAVE_rs6000_xcoff_vec)

{ "powerpc-*-aix[5-9]*", NULL },{ "rs6000-*-aix[5-9]*",
&rs6000_xcoff_vec },
#endif



    
#ifdef BFD64

#if !defined (SELECT_VECS) || defined (HAVE_rs6000_xcoff64_aix_vec)

{ "powerpc64-*-aix[5-9]*", NULL },{ "rs6000-*-aix[5-9]*",
&rs6000_xcoff64_aix_vec },
#endif



    
#endif

#if !defined (SELECT_VECS) || defined (HAVE_rs6000_xcoff_vec)

{ "powerpc-*-aix*", NULL },{ "powerpc-*-beos*", NULL },{ "rs6000-*-*",
&rs6000_xcoff_vec },
#endif


    
#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_rs6000_xcoff64_vec)

{ "powerpc64-*-aix*",
&rs6000_xcoff64_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_elf64_fbsd_vec)

{ "powerpc64-*-freebsd*",
&powerpc_elf64_fbsd_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_elf64_vec)

{ "powerpc64-*-elf*", NULL },{ "powerpc-*-elf64*", NULL },{ "powerpc64-*-linux*", NULL },
{ "powerpc64-*-*bsd*",
&powerpc_elf64_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_elf64_le_vec)

{ "powerpc64le-*-elf*", NULL },{ "powerpcle-*-elf64*", NULL },{ "powerpc64le-*-linux*", NULL },
{ "powerpc64le-*-*bsd*",
&powerpc_elf64_le_vec },
#endif



    
#endif
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_elf32_fbsd_vec)

{ "powerpc-*-*freebsd*",
&powerpc_elf32_fbsd_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_elf32_vec)

{ "powerpc-*-*bsd*", NULL },{ "powerpc-*-elf*", NULL },{ "powerpc-*-sysv4*", NULL },{ "powerpc-*-eabi*", NULL },
{ "powerpc-*-solaris2*", NULL },{ "powerpc-*-linux-*", NULL },{ "powerpc-*-rtems*", NULL },
{ "powerpc-*-chorus*",
&powerpc_elf32_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_elf32_vec)

{ "powerpc-*-kaos*",
&powerpc_elf32_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_mach_o_be_vec)

{ "powerpc-*-darwin*", NULL },{ "powerpc-*-macos10*", NULL },{ "powerpc-*-rhapsody*",
&mach_o_be_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_xcoff_vec)

{ "powerpc-*-macos*",
&powerpc_xcoff_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_elf32_vec)

{ "powerpc-*-lynxos*",
&powerpc_elf32_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_elf32_vec)

{ "powerpc-*-netware*",
&powerpc_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_elf32_vec)

{ "powerpc-*-nto*",
&powerpc_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_elf32_vxworks_vec)

{ "powerpc-*-vxworks*", NULL },{ "powerpc-*-windiss*",
&powerpc_elf32_vxworks_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_elf32_le_vec)

{ "powerpcle-*-nto*",
&powerpc_elf32_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_elf32_le_vec)

{ "powerpcle-*-elf*", NULL },{ "powerpcle-*-sysv4*", NULL },{ "powerpcle-*-eabi*", NULL },
{ "powerpcle-*-solaris2*", NULL },{ "powerpcle-*-linux-*", NULL },{ "powerpcle-*-vxworks*",
&powerpc_elf32_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_powerpc_pe_le_vec)

{ "powerpcle-*-pe", NULL },{ "powerpcle-*-winnt*", NULL },{ "powerpcle-*-cygwin*",
&powerpc_pe_le_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_rl78_elf32_vec)

{ "rl78-*-elf",
&rl78_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_rx_elf32_le_vec)

{ "rx-*-elf",
&rx_elf32_le_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_s390_elf32_vec)

{ "s390-*-linux*",
&s390_elf32_vec },
#endif



    
#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_s390_elf64_vec)

{ "s390x-*-linux*",
&s390_elf64_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_s390_elf64_vec)

{ "s390x-*-tpf*",
&s390_elf64_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_score_elf32_be_vec)

{ "score*-*-elf*",
&score_elf32_be_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_sh64_elf32_le_vec)

{ "sh64l*-*-elf*",
&sh64_elf32_le_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_sh64_elf32_vec)

{ "sh64-*-elf*",
&sh64_elf32_vec },
#endif




    
#if !defined (SELECT_VECS) || defined (HAVE_sh64_elf32_linux_be_vec)

{ "sh64eb-*-linux*",
&sh64_elf32_linux_be_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sh64_elf32_linux_vec)

{ "sh64-*-linux*",
&sh64_elf32_linux_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_linux_be_vec)

{ "sh-*-linux*",
&sh_elf32_linux_be_vec },
#endif



    
#endif /* BFD64 */

#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_linux_be_vec)

{ "sh*eb-*-linux*",
&sh_elf32_linux_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_linux_vec)

{ "sh*-*-linux*",
&sh_elf32_linux_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_vec)

{ "sh-*-uclinux*", NULL },{ "sh[12]-*-uclinux*",
&sh_elf32_vec },
#endif


#ifdef BFD64

#endif
    

#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_sh64_elf32_nbsd_le_vec)

{ "sh5le-*-netbsd*",
&sh64_elf32_nbsd_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sh64_elf32_nbsd_vec)

{ "sh5-*-netbsd*",
&sh64_elf32_nbsd_vec },
#endif



    

#if !defined (SELECT_VECS) || defined (HAVE_sh64_elf64_nbsd_le_vec)

{ "sh64le-*-netbsd*",
&sh64_elf64_nbsd_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sh64_elf64_nbsd_vec)

{ "sh64-*-netbsd*",
&sh64_elf64_nbsd_vec },
#endif



    

#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_nbsd_le_vec)

{ "sh*l*-*-netbsdelf*",
&sh_elf32_nbsd_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_nbsd_vec)

{ "sh-*-netbsdelf*",
&sh_elf32_nbsd_vec },
#endif



    
#endif

#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_nbsd_vec)

{ "sh*-*-netbsdelf*",
&sh_elf32_nbsd_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_symbian_le_vec)

{ "sh*-*-symbianelf*",
&sh_elf32_symbian_le_vec },
#endif



    

#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_le_vec)

{ "shl*-*-elf*", NULL },{ "sh[1234]l*-*-elf*", NULL },{ "sh3el*-*-elf*", NULL },{ "shl*-*-kaos*",
&sh_elf32_le_vec },
#endif




    
#endif

#if !defined (SELECT_VECS) || defined (HAVE_sh_coff_vec)

{ "sh-*-rtemscoff*",
&sh_coff_vec },
#endif



    

#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_vec)

{ "sh-*-elf*", NULL },{ "sh[1234]*-elf*", NULL },{ "sh-*-rtems*", NULL },{ "sh-*-kaos*",
&sh_elf32_vec },
#endif




    
#endif

#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_vec)

{ "sh-*-nto*",
&sh_elf32_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_nbsd_le_vec)

{ "sh*-*-openbsd*",
&sh_elf32_nbsd_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sh_pe_le_vec)

{ "sh-*-pe",
&sh_pe_le_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sh_elf32_vxworks_vec)

{ "sh-*-vxworks",
&sh_elf32_vxworks_vec },
#endif







    
#if !defined (SELECT_VECS) || defined (HAVE_sh_coff_vec)

{ "sh-*-*",
&sh_coff_vec },
#endif



    

#if !defined (SELECT_VECS) || defined (HAVE_sparc_aout_sunos_be_vec)

{ "sparclet-*-aout*",
&sparc_aout_sunos_be_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_aout_sunos_be_vec)

{ "sparc86x-*-aout*",
&sparc_aout_sunos_be_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf32_vec)

{ "sparclite-*-elf*", NULL },{ "sparc86x-*-elf*",
&sparc_elf32_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf32_vec)

{ "sparc*-*-chorus*",
&sparc_elf32_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_aout_linux_vec)

{ "sparc-*-linux*aout*",
&sparc_aout_linux_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf32_vec)

{ "sparc-*-linux-*", NULL },{ "sparcv*-*-linux-*",
&sparc_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf32_vec)

{ "sparc-*-netbsdelf*",
&sparc_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_aout_nbsd_vec)

{ "sparc-*-netbsdaout*", NULL },{ "sparc-*-netbsd*",
&sparc_aout_nbsd_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_aout_nbsd_vec)

{ "sparc-*-openbsd[0-2].*", NULL },{ "sparc-*-openbsd3.[0-1]",
&sparc_aout_nbsd_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf32_vec)

{ "sparc-*-openbsd*",
&sparc_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf32_vec)

{ "sparc-*-elf*",
&sparc_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf32_sol2_vec)

{ "sparc-*-solaris2.[0-6]", NULL },{ "sparc-*-solaris2.[0-6].*",
&sparc_elf32_sol2_vec },
#endif


    
#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf32_sol2_vec)

{ "sparc-*-solaris2*", NULL },{ "sparcv9-*-solaris2*", NULL },{ "sparc64-*-solaris2*",
&sparc_elf32_sol2_vec },
#endif



    
#endif
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf32_vec)

{ "sparc-*-sysv4*",
&sparc_elf32_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf32_vxworks_vec)

{ "sparc-*-vxworks*",
&sparc_elf32_vxworks_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf32_vec)

{ "sparc-*-netware*",
&sparc_elf32_vec },
#endif


    
#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_sparc_aout_sunos_be_vec)

{ "sparc64-*-aout*",
&sparc_aout_sunos_be_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf64_vec)

{ "sparc64*-*-linux-*",
&sparc_elf64_vec },
#endif



    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf64_vec)

{ "sparc64-*-elf*", NULL },{ "sparc64-*-rtems*",
&sparc_elf64_vec },
#endif



    
#endif /* BFD64 */
#if !defined (SELECT_VECS) || defined (HAVE_sparc_coff_vec)

{ "sparc*-*-coff*",
&sparc_coff_vec },
#endif

    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_elf32_vec)

{ "sparc-*-rtems*",
&sparc_elf32_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_sparc_aout_sunos_be_vec)

{ "sparc*-*-*",
&sparc_aout_sunos_be_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_spu_elf32_vec)

{ "spu-*-elf",
&spu_elf32_vec },
#endif


    

#if HAVE_aout_vec
#if !defined (SELECT_VECS) || defined (HAVE_aout_vec)

{ "tahoe-*-*",
&aout_vec },
#endif


    
#endif

#if !defined (SELECT_VECS) || defined (HAVE_tic6x_elf32_c6000_le_vec)

{ "tic6x-*-elf",
&tic6x_elf32_c6000_le_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_tic6x_elf32_linux_le_vec)

{ "tic6x-*-uclinux",
&tic6x_elf32_linux_le_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_tic80_coff_vec)

{ "tic80*-*-*",
&tic80_coff_vec },
#endif


    

#ifdef BFD64
#if !defined (SELECT_VECS) || defined (HAVE_tilegx_elf64_le_vec)

{ "tilegx-*-*",
&tilegx_elf64_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_tilegx_elf64_be_vec)

{ "tilegxbe-*-*",
&tilegx_elf64_be_vec },
#endif


    
#endif

#if !defined (SELECT_VECS) || defined (HAVE_tilepro_elf32_vec)

{ "tilepro-*-*",
&tilepro_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_v850_elf32_vec)

{ "v850*-*-*",
&v850_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_vax_elf32_vec)

{ "vax-*-netbsdelf*",
&vax_elf32_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_vax_aout_nbsd_vec)

{ "vax-*-netbsdaout*", NULL },{ "vax-*-netbsd*",
&vax_aout_nbsd_vec },
#endif



    

#if !defined (SELECT_VECS) || defined (HAVE_vax_aout_bsd_vec)

{ "vax-*-bsd*", NULL },{ "vax-*-ultrix*",
&vax_aout_bsd_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_vax_aout_nbsd_vec)

{ "vax-*-openbsd*",
&vax_aout_nbsd_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_vax_elf32_vec)

{ "vax-*-linux-*",
&vax_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_we32k_coff_vec)

{ "we32k-*-*",
&we32k_coff_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_w65_coff_vec)

{ "w65-*-*",
&w65_coff_vec },
#endif

    
    
#if !defined (SELECT_VECS) || defined (HAVE_xgate_elf32_vec)

{ "xgate-*-*",
&xgate_elf32_vec },
#endif


    
 	
#if !defined (SELECT_VECS) || defined (HAVE_xstormy16_elf32_vec)

{ "xstormy16-*-elf",
&xstormy16_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_xtensa_elf32_le_vec)

{ "xtensa*-*-*",
&xtensa_elf32_le_vec },
#endif


    
#if !defined (SELECT_VECS) || defined (HAVE_xc16x_elf32_vec)

{ "xc16x-*-elf",
&xc16x_elf32_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_z80_coff_vec)

{ "z80-*-*",
&z80_coff_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_z8k_coff_vec)

{ "z8k*-*-*",
&z8k_coff_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_ieee_vec)

{ "*-*-ieee*",
&ieee_vec },
#endif

    

#if !defined (SELECT_VECS) || defined (HAVE_aout_adobe_vec)

{ "*-adobe-*",
&aout_adobe_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_m68k_aout_newsos3_vec)

{ "*-sony-*",
&m68k_aout_newsos3_vec },
#endif


    

#if !defined (SELECT_VECS) || defined (HAVE_m68k_coff_vec)

{ "*-tandem-*",
&m68k_coff_vec },
#endif


    
