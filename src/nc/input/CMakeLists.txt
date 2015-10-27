if(LIBBFD_FOUND)
	add_subdirectory(bfd)
else()
	add_subdirectory(elf)
	add_subdirectory(mach-o)
	add_subdirectory(pe)
endif()

# vim:set et sts=4 sw=4 nospell:
