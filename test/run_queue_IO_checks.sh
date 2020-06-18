


if [[ ! -d ./build ]]; then

cat<<EOF >&2

ERROR: you need to be in the base directory of your installation to run this script. Returning...

EOF

return 1

fi

if [[ -z $DUNE_DAQ_BUILD_SCRIPT_SOURCED ]]; then

    cat<<EOF >&2

WARNING: the environment variable DUNE_DAQ_BUILD_SCRIPT_SOURCED does
not appear to be set, indicating that the "source_me_to_build" script
has not been sourced (successfully) and hence environmental
assumptions behind this script may not hold. The attempt to run checks
on our queues may not work...

EOF

fi

for queue_type in FollyMPMCQueue FollyMPMCQueuePopT FollySPSCQueue FollySPSCQueuePopT StdDeQueue ; do
echo
echo "One thread pushing elements onto the queue, no threads popping, ${queue_type}"
echo
./build/appfwk/test/queue_IO_check --push_threads 1 --pop_threads 0 --initial_capacity_used 0 --queue_type ${queue_type}
done

for queue_type in FollyMPMCQueue FollyMPMCQueuePopT FollySPSCQueue FollySPSCQueuePopT StdDeQueue ; do
echo
echo "One thread popping elements off the queue, no threads pushing, ${queue_type}"
echo
./build/appfwk/test/queue_IO_check --pop_threads 1 --push_threads 0 --initial_capacity_used 0.6 --queue_type ${queue_type}
done

for queue_type in FollyMPMCQueue FollyMPMCQueuePopT FollySPSCQueue FollySPSCQueuePopT StdDeQueue ; do
echo
echo "One thread pushing elements onto the queue, one thread popping, ${queue_type}"
echo
./build/appfwk/test/queue_IO_check --pop_threads 1 --push_threads 1 --initial_capacity_used 0.5 --queue_type ${queue_type}
done

for queue_type in FollyMPMCQueue FollyMPMCQueuePopT StdDeQueue ; do
echo
echo "Two threads pushing elements onto the queue, two thread popping, ${queue_type}"
echo
./build/appfwk/test/queue_IO_check --pop_threads 2 --push_threads 2 --initial_capacity_used 0.5 --queue_type ${queue_type}
done

