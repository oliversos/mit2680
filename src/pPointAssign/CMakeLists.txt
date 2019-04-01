#--------------------------------------------------------
# The CMakeLists.txt for:                       pPointAssign
# Author(s):                              Oliver Os
#--------------------------------------------------------

SET(SRC
  PointAssign.cpp
  PointAssign_Info.cpp
  main.cpp
  Point.cpp
)

ADD_EXECUTABLE(pPointAssign ${SRC})

TARGET_LINK_LIBRARIES(pPointAssign
   ${MOOS_LIBRARIES}
   geometry
   mbutil
   m
   pthread)

