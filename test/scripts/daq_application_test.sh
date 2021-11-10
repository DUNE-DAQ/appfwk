
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

echo
echo "INFO: Testing \"daq_application -h\""
echo

daq_application -h
ret=$?
if [ $ret -ne 0 ]; then
  echo
  echo "WARNING: Unexpected return code $ret from \"daq_application -h\""
  echo
fi

echo
echo "INFO: Testing daq_application without options"
echo

daq_application
ret=$?
if [ $ret -ne 255 ]; then
  echo
  echo "WARNING: Unexpected return code $ret from daq_application without options"
  echo
fi

echo
echo "INFO: Testing daq_application with stdin command facility"
echo

echo "status"|daq_application --name "Test App" -c "stdin://${SCRIPT_DIR}/test.json"
ret=$?

if [ $ret -ne 0 ]; then
  echo
  echo "WARNING: Unexpected return code $ret from daq_application with stdin command facility"
  echo
fi

echo
echo "INFO: Testing daq_application signal handling"
echo

mkfifo input && trap "rm input" EXIT
tail -f input|daq_application --name "Test App" -c "stdin://${SCRIPT_DIR}/test.json" & PID=$!

echo "init" >input
sleep 1

echo "Sending SIGQUIT"
kill -SIGQUIT $PID
sleep 1

echo "conf" >input
sleep 1

echo "start" >input # This should not execute!
sleep 1

fuser -TERM -k input
wait $PID
ret=$?

if [ $ret -ne 0 ]; then
  echo
  echo "WARNING: Unexpected return code $ret from daq_application signal handling"
  echo
fi



