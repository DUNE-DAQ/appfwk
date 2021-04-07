In the *appfwk* package, an executable for queue testing called `queue_IO_check` exists which writes and reads integers to instances of classes which implement the *appfwk*'s `Queue` interface class. It has several adjustable parameters, including the number of threads doing the writing, the number of threads doing the reading, the number of integers you want to write/read, and the type of queue you want to test. 

To use it, you'll want to set up the *appfwk* environment as described [[here|Compiling-and-running]]. `queue_IO_check` has several adjustable parameters which take on defaults, but for starters you can run it without arguments. If you're in the base of your development area you can just do the following:
```
queue_IO_check 
```
What this will do is take an empty instance of the `StdDeQueue` class, and have one thread writing (pushing) one million integers onto the queue with another thread reading (popping) them. When you're done, you'll see something like the following:
```
08-19 15:45:02.537972   TRACE nfo 1 thread(s) pushing 1000000 elements between them, each thread has an average time of 0 milliseconds between pushes
08-19 15:45:02.538080   TRACE nfo 1 thread(s) popping 1000000 elements between them, each thread has an average time of 0 milliseconds between pops
08-19 15:45:02.538086   TRACE nfo Queue of type StdDeQueue has capacity for 1000000000 elements
08-19 15:45:02.538102   TRACE wrn The number of elements the queue is initially filled with (0) minus the number of elements which will be popped off of it (1000000) is less than zero
08-19 15:45:03.470687   TRACE nfo Thread #140181788210944: started at 1597869902558 ms since epoch, tried pushing 1000000 elements; time taken was 912 ms
08-19 15:45:03.525529   TRACE nfo Thread #140181779818240: started at 1597869902558 ms since epoch, tried popping 1000000 elements; time taken was 967 ms
08-19 15:45:03.525628   TRACE nfo 

Final results: 
08-19 15:45:03.525697   TRACE nfo Max queue size during running was 256848
08-19 15:45:03.525724   TRACE nfo There were 0 exception throws on push calls
08-19 15:45:03.525754   TRACE nfo There were 0 pushes which took longer than the provided timeout of 100 ms
08-19 15:45:03.525769   TRACE nfo There were 0 exception throws on pop calls
08-19 15:45:03.525783   TRACE nfo There were 0 pops which took longer than the provided timeout of 100 ms
08-19 15:45:03.525796   TRACE nfo Total time from start of thread launch to the last thread wrapping up was 967 ms
```
In this particular instance, we see that the pushing/popping took just under a second, and that none of the push or pop calls threw an exception (typically because of a timeout) or took longer than the expected timeout. The warning near the top about the user attempting to pop more elements than are initially on the queue is simply meant to alert the tester that the time the popping thread takes could in principle be affected by it waiting for the pushing thread to add elements to an empty queue. 

Optional arguments you'll likely supply can be listed if you run
```
queue_IO_check --help
```
And the ones you'll find most useful are the following:
```
--queue_type <type>
```
...where `<type>` can be `FollySPSCQueue`, `FollyMPMCQueue`, and `StdDeQueue`. These options will increase as new implementations of `Queue` become available. 
```
--push_threads <# of threads>
```
...which is the number of threads pushing. This must be >=0, though in the case of `FollySPSCQueue` it can only be 0 or 1. If set to 0, this means you want only to see how long it takes to pop elements off of an already-filled queue; how to do this will be shown below. 
```
--pop_threads <# of threads>
```
...which is the number of threads popping. This must be >=0; setting this to 0 means simply that you just want to see how long it takes to push the elements onto the queue without additional threads removing the elements. In the case of `FollySPSCQueue` it can only be 0 or 1. 
```
--nelements <# of elements>
```
This will specify the number of elements which get pushed/popped onto/off of the queue. Must be >=1. If you've selected 0 popping threads, this must not exceed the capacity of the queue (default value is 1 billion, and it's unlikely you'd ever want to exceed this). 
```
--initial_capacity_used <fraction of capacity>
```
This will fill the queue in advance with the provided fraction of its initial capacity. E.g., with a capacity of 1 billion, setting `<fraction of capacity>` to `0.01` will fill the queue with ten million elements before setting the timer and starting the test. If you wish to test popping only, obviously you'll need to set this. 

Note that when testing it can be convenient to use loops. E.g., you could do the following:
```
for queue in FollyMPMCQueue StdDeQueue ; do
  echo
  echo "Testing $queue:" 
  ./build/appfwk/test/queue_IO_check --push_threads 4 --pop_threads 1 --initial_capacity_used 0.001 --queue $queue 

done
```
if you wanted to see how the queues behaved with four threads trying to write data to them at once while just one thread read from them. You could also put such code in a bash script, to run it later. 
