#!/bin/bash -e
COMMUNITY1="alpha"
COMMUNITY2="bravo"
COMMUNITY3="shoreside"

#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
TIME_WARP=1
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
	printf "%s [SWITCHES] [time_warp]   \n" $0
	printf "  --help, -h         \n" 
	exit 0;
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$TIME_WARP" = 1 ]; then 
        TIME_WARP=$ARGI
    else 
	printf "Bad Argument: %s \n" $ARGI
	exit 0
    fi
done


#-------------------------------------------------------
#  Part 2: Launch the processes
#-------------------------------------------------------

#alpha
printf "Launching the %s MOOS Community (WARP=%s) \n"  $COMMUNITY1 $TIME_WARP
pAntler $COMMUNITY1.moos --MOOSTimeWarp=$TIME_WARP >& /dev/null &
printf "Launching the %s MOOS Community (WARP=%s) \n"  $COMMUNITY2 $TIME_WARP
pAntler $COMMUNITY2.moos --MOOSTimeWarp=$TIME_WARP >& /dev/null &
printf "Launching the %s MOOS Community (WARP=%s) \n"  $COMMUNITY3 $TIME_WARP
pAntler $COMMUNITY3.moos --MOOSTimeWarp=$TIME_WARP >& /dev/null &

uMAC -t $COMMUNITY3.moos 

printf "Killing all processes ... \n"
kill %1 
mykill
printf "Done killing processes.   \n"

#bravo
#printf "Launching the %s MOOS Community (WARP=%s) \n"  $COMMUNITY2 $TIME_WARP
#pAntler $COMMUNITY2.moos --MOOSTimeWarp=$TIME_WARP >& /dev/null &
#
#uMAC -t $COMMUNITY2.moos
#
#printf "Killing all processes ... \n"
#kill %1 
#mykill
#printf "Done killing processes.   \n"




