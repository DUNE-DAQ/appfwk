
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

function run_test() {

# Run without options
queue_IO_check $@
ret=$?
if [ $ret -ne 0 ]; then
  echo
  echo "WARNING: Unexpected return code $ret from \"queue_IO_check $@\""
  echo
  return 1
fi
return 0
}

num_failures=0
num_tests=0
for queue_type in StdDeQueue FollySPECQueue FollyMPMCQueue; do
    run_test --queue_type $queue_type --nelements 1000000 --push_threads 1 --pop_threads 1 --pause_between_pushes 0 --pause_between_pops 0 --capacity 1000000000 --initial_capacity_used 0
    num_tests=$(($num_tests + 1))
    num_failures=$(($num_failures + $?))
    run_test --queue_type $queue_type --nelements 1000 --push_threads 10 --pop_threads 1 --pause_between_pushes 0 --pause_between_pops 0 --capacity 10000 --initial_capacity_used 0
    num_tests=$(($num_tests + 1))
    num_failures=$(($num_failures + $?))
    run_test --queue_type $queue_type --nelements 1000 --push_threads 1 --pop_threads 10 --pause_between_pushes 0 --pause_between_pops 0 --capacity 10000 --initial_capacity_used 0
    num_tests=$(($num_tests + 1))
    num_failures=$(($num_failures + $?))
    run_test --queue_type $queue_type --nelements 1000 --push_threads 10 --pop_threads 10 --pause_between_pushes 0 --pause_between_pops 0 --capacity 10000 --initial_capacity_used 0
    num_tests=$(($num_tests + 1))
    num_failures=$(($num_failures + $?))
    run_test --queue_type $queue_type --nelements 1000 --push_threads 1 --pop_threads 1 --pause_between_pushes 100 --pause_between_pops 100 --capacity 10000 --initial_capacity_used 0
    num_tests=$(($num_tests + 1))
    num_failures=$(($num_failures + $?))
    run_test --queue_type $queue_type --nelements 1000 --push_threads 1 --pop_threads 1 --pause_between_pushes 0 --pause_between_pops 0 --capacity 10000 --initial_capacity_used 0.5
    num_tests=$(($num_tests + 1))
    num_failures=$(($num_failures + $?))
    run_test --queue_type $queue_type --nelements 1000 --push_threads 1 --pop_threads 1 --pause_between_pushes 0 --pause_between_pops 0 --capacity 10000 --initial_capacity_used 1
    num_tests=$(($num_tests + 1))
    num_failures=$(($num_failures + $?))
done

echo "Completed $num_tests tests with $num_failures failures"