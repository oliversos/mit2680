#--------------------------------------------------------
# The CMakeLists.txt for:                       pGenPath
# Author(s):                              Oliver Os
#--------------------------------------------------------

SET(SRC
  GenPath.cpp
  GenPath_Info.cpp
  main.cpp
  ../pPointAssign/Point.cpp
)

ADD_EXECUTABLE(pGenPath ${SRC})

TARGET_LINK_LIBRARIES(pGenPath
   ${MOOS_LIBRARIES}
   geometry
   mbutil
   m
   pthread)

