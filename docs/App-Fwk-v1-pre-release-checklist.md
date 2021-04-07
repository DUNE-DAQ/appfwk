# The list of action items to complete before releasing v1:

* [Kurt] create an example 'user' package that has a small number of example DAQModules and the JSON configuration file that should be used to create a DAQ application that uses them. [Issue #52](https://github.com/DUNE-DAQ/appfwk/issues/52)

* [Marco/Eric] Review wiki pages - delete obsolete pages and check that others are up-to-date [Issue #53](https://github.com/DUNE-DAQ/appfwk/issues/53)

* Finish Folly Queue implementation integration [Pull Request #40](https://github.com/DUNE-DAQ/appfwk/pull/40)

* Consistently replace trace and std::exception with ers [Issue #54](https://github.com/DUNE-DAQ/appfwk/issues/54)

* [Alessandro] Change execute_command to a register command with callback interface ? [Issue #55](https://github.com/DUNE-DAQ/appfwk/issues/55)

* [Phil/Pengfei] Include UPS-ified folly dependencies in cvmfs. Move/copy Phil's repo to generate the dependencies under DUNE DAQ. [Issue #61](https://github.com/DUNE-DAQ/appfwk/issues/61)

* document the list of items that we knowingly deferred in v1.  This list is a candidate for a Wiki page and a README file in the appfwk repo. [Issue #57](https://github.com/DUNE-DAQ/appfwk/issues/57)
  * Candidate topics:
    * the Queue pop() interface

* [DONE] depending on the outcome of Alessandro's investigation into registering commands from DAQModules, possibly remove echo_test_app and DebugLoggingDAQModule from the appfwk repo.

* items from our 17-June discussion that are not covered elsewhere in this list:
  * benchmark the performance of the queues that we have in v1 (and document that performance in the release notes?)
  * [DONE] Merge the latest single-CMakeLists.txt changes into the "develop" branch [Issue #34](https://github.com/DUNE-DAQ/appfwk/issues/34)
  * [DONE] remove test/producer_consumer_test_app.cxx from the appfwk repo
  * add the setting of the cet_plugin_path env var to the source_me_to_build script

* documentation notes:
  * [from Kurt] [Issue #50](https://github.com/DUNE-DAQ/appfwk/issues/50) In trying to create the top-level CMakeLists.txt file for the example user package, it would be helpful to have a medium-level guide to what is in the standard CMakeLists.txt file.  This could take the form of comments in the file, or a Wiki page that describes the different sections of the file.  Some information that I would find useful:
    * what are the functions of the point_build_to() and MakeDataTypeLibraries() macros
    * what are the scope(s) of the various set, add_xyz, and target_link_libraries calls (I'm guessing that their scope is the most-recent call to point_build_to(), but it would be nice to have that verified.
    * in writing this, I realize that there are a number of ways to organize or view the the documentation that I'm thinking about.  One is an explanation of the parts of the CMakeLists.txt file that is used in the appfwk repo.  That might be useful as part of a tutorial on the appfwk and/or software coordination.  A second type of documentation would be reference information on each of the CMake macros that are part of the DUNE DAQ environment, what they do, how to use them, etc.  A third type of documentation would describe the elements of the CMakeLists.txt file that users should create in their user-DAQModule packages.  Once I understand the parts of the CMakeLists.txt file that is used in the appfwk repo, I'm sure that I can create this third type of documentation, but it would be great to have the first type of documentation to help me do that.
  * reminder(s) for user example documentation
    * explain what users see on the console when they run source_me_to_build (e.g. "nlohmann_json NOT FOUND!")
    * explain how to run "-j 1" if there are lots of errors, so that the error messages can be more easily interpreted

* Questions:
  * [From Kurt] There are a number of warnings when we run source_me_to_build (mainly unused parameters).  Should we do something to remove those warnings (e.g. change the code that is being warned about) or just document what users should overlook?  I suspect the former...
  * [DONE] [From Kurt] Why does "build/appfwk/apps/daq_application -c QueryResponseCommandFacility -j appfwk/test/producer_consumer_dynamic_test.json" crash with a Segmentation fault after being given a "start" command? [Issue #45](https://github.com/DUNE-DAQ/appfwk/issues/45)
