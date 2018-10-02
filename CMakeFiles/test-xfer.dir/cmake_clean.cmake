file(REMOVE_RECURSE
  "test-xfer.pdb"
  "test-xfer"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/test-xfer.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
