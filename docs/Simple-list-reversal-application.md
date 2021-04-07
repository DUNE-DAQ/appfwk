28-Apr-2020, KAB: the main points of this simple application are to demonstrate the following:
* how we create UserModules and Buffers/Queues, and how we connect them together
* how we can specify the order that a Start command is passed to the UserModules, and how we can specify a different order for a Stop command

This sample is all contained inside one DAQProcess, and it has three UserModules.
* the first UserModule (the Generator) creates a new list of 4 random integers every second and puts copies of that list onto two queues (a PrimaryData queue and a DataCopy queue)  [The use of names here for UserModules and Buffers/Queues is just for convenience in this description.  It does not imply that these entities in the DAQProcess need to be referenced by any such name.]
* the second UserModule (the Reverser) reads each list off the PrimaryData queue, reverses the order of the elements, and pushes the result onto the ReversedData queue.
* the third UserModule (the Validator) reads each list off the ReversedData queue, reads the corresponding original list off the DataCopy queue, and compares the results.  There should be a way for the Validator to inform the user of the success or failure of each comparison.  This can be as simple as a message on the console.

Thinking about object lifetime...  My understanding is that all of the UserModules and all of the Buffers/Queues will be created at application start.  The command-line CommandFacility that is currently implemented will be used to specify a Start command and a Stop command.  The Start command should be passed to the Validator UserModule first, the Reverser UM second, and the Generator UM third.  The Stop command should be passed to the Generator UM first, the Reverser UM second, and the Validator UM third.  
Upon receiving the Stop command, the Generator UM will finish any generation that it has in progress and then stop generating lists of integers.  The time that it takes for the Generator UM to process a Stop command should be less than 1 second.  That is, the list generator thread(?) should check for a stop more frequently than once per second.  
When the Reverser UM receives the Stop command, it should finish processing any lists that are on the PrimaryData queue and then stop looking for new lists.  It should also take less than 1 second to complete the Stop operation.
The Validator UM should also finish any pending lists and stop in times less than 1 second.

I'll try to create a diagram that shows the different entities in this example.

Possible enhancements
* make the 1-second generation interval configurable
* make the size of the data lists that are generated, reversed, and validated configurable
