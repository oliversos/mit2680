#--------------------------------------------------------
# The CMakeLists.txt for:                       uFldHazardMgrX
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  Classification.cpp
  HazardMgr.cpp
  HazardMgr_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(uFldHazardMgrX ${SRC})

TARGET_LINK_LIBRARIES(uFldHazardMgrX
   ${MOOS_LIBRARIES}
   ufld_hazards
   geometry
   apputil
   mbutil
   ${SYSTEM_LIBS}
)

