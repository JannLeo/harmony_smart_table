. = ALIGN(8); __block_sysmemused_start = .; __sysmemused_start = .; KEEP(*(.sysmemused)); __sysmemused_end = .; . += (__sysmemused_end - __sysmemused_start)*(((32 + 1)) - 1); __block_sysmemused_end = .;
. = ALIGN(8); __block_errno_start = .; __errno_start = .; KEEP(*(.errno)); __errno_end = .; . += (__errno_end - __errno_start)*(((32 + 1)) - 1); __block_errno_end = .;
