_JCF, Nov-6-2020: for v1.2.1, it was discovered in the last day or two that you need your current directory in your PATH variable. When logging in to a shell, please execute "export PATH=.:${PATH}". This will not be necessary in later daq-buildtools versions._

To get set up, you'll need access to the ups product area `/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products`, as is the case, e.g., on the lxplus machines at CERN. If you're on a system which has access to this product area, simply do the following after you've logged in to your system and created an empty directory (we'll refer to it as "MyTopDir" on this wiki):
``` 
curl -O https://raw.githubusercontent.com/DUNE-DAQ/daq-buildtools/v1.2.1/bin/quick-start.sh
chmod +x quick-start.sh
./quick-start.sh
```
which will set up an area to place the repos you wish to build, `./sourcecode`, provide you with a file you can source to set up the build environment, `setup_build_environment`, and an executable script you can run for builds, `build_daq_software.sh`.

If you look in `./sourcecode` you'll see there's a repo already downloaded, `./sourcecode/daq-buildtools`. This will be needed in order to build any other standard DUNE DAQ packages. For the purposes of instruction - as well as the fact that it's very likely that your own package will depend on it - let's also build the appfwk package. Downloading the appfwk is simple:
```
cd sourcecode
git clone https://github.com/DUNE-DAQ/appfwk.git
git clone https://github.com/DUNE-DAQ/cmdlib.git
cd ..
```
Note that in those commands we not only downloaded the appfwk repo but also the cmdlib repo; this is because appfwk depends on cmdlib, and this package isn't (yet, Oct-15-2020) available via ups. 

We're about to build and install daq-buildtools, the cmdlib package which depends on it, and the appfwk package which in turn depends on cmdlib. By default, the scripts will create a subdirectory of MyTopDir called ./install and install the packages there. If you wish to install them in another location, you'll want to set the environment variable DBT_INSTALL_DIR to the desired installation path before taking any further action.

Now, do the following:
```
. ./setup_build_environment  # Only needs to be done once in a given shell
./build_daq_software.sh --install
```
...and this will build daq-buildtools, cmdlib and appfwk in the local `./build` subdirectory and then install them as packages either in the local `./install` subdirectory or in whatever you pointed DBT_INSTALL_DIR to. 

To work with more repos, add them to the `./sourcecode` subdirectory as we did with appfwk. Be aware, though: if you're developing a new repo which itself depends on another new repo, daq-buildtools may not already know about this dependency. "New" in this context means "not found on https://github.com/DUNE-DAQ as of Oct-15-2020". If this is the case, you have one of two options:

* (Recommended) Add the names of your new packages to the `build_order` list found near the bottom of `./sourcecode/CMakeLists.txt`, placing them in the list in the relative order in which you want them to be built. 
* First clone, build and install your new base repo, and THEN clone, build and install your other new repo which depends on your new base repo. 

`build_daq_software.sh` will by default skip CMake's config+generate stages and go straight to the build stage _unless_ either the `CMakeCache.txt` file isn't found in `./build` or you've just added a new repo to `./sourcecode`. If you want to remove all the contents of `./build` and run config+generate+build, all you need to do is add the `--clean` option, i.e.
```
./build_daq_software.sh --clean --install
```
And if, after the build, you want to run the unit tests, just add the `--unittest` option. Note that it can be used with or without `--clean`, so, e.g.:
```
./build_daq_software.sh --clean --install --unittest  # Blow away the contents of ./build, run config+generate+build, and then run the unit tests
```
..where in the above case, you blow away the contents of ./build,  run config+generate+build, install the result in `./install` and then run the unit tests.

To check for deviations from the coding rules described in the [DUNE C++ Style Guide](https://github.com/DUNE-DAQ/styleguide/blob/develop/dune-daq-cppguide.md), run with the `--lint` option:
```
./build_daq_software.sh --lint
```
...though be aware that some guideline violations (e.g., having a function which tries to do unrelated things) can't be picked up by the automated linter. 

If you want to see verbose output from the compiler, all you need to do is add the `--verbose` option:
```
./build_daq_software.sh --verbose 
```

You can see all the options listed if you run the script with the `--help` command, i.e.
```
./build_daq_software.sh --help
```
Finally, note that both the output of your builds and your unit tests are logged to files in the `./log` subdirectory. These files may have ASCII color codes which make them difficult to read with some tools; `more` or `cat`, however, will display the colors and not the codes themselves. 

</details>

## Running

In order to run the applications built during the above procedure, the system needs to be instructed on where to look for the libraries that can be used to instantiate objects. This is handled by the `./setup_runtime_environment` script which was placed in MyTopDir when you ran quick-start.sh; all you need to do is source it:
```
. ./setup_runtime_environment
```

Note that if you add a new repo to your development area, after building your new code you'll need to source the script again. 

Once the runtime environment is set, just run the application you need.  

We're now going to go through a demo in which we'll send vectors from appfwk's FakeDataProducer DAQ module to its FakeDataConsumer DAQ module. The package that interface (cmdlib) the modules with the command sending functionality, is already cloned. It comes with a basic implementation that is capable of sending available command objects from a pre-loaded file, by typing their command IDs to standard input. This command facility is useful for local, test oriented use-cases. In the same runtime area, launch the application like this:
```
daq_application -c stdin://sourcecode/appfwk/schema/fdpc-job.json
```

For a more realistic use-case, where you can send commands to the application from other services and applications, the [restcmd](https://github.com/DUNE-DAQ/restcmd) library provides a command handling implementation through HTTP. To use this plugin, we'll want to obtain and build it. Since restcmd relies on the pistache package which isn't automatically set up in `setup_build_environment`, we'll want to first set up pistache:
```
. /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products_dev/setup
setup pistache v2020_10_07 -q e19:prof
```
You may want to add these two commands to your copy of `setup_build_environment` so you don't need to worry about them in the future. Now let's clone and build restcmd the way we would any other package. Assuming we're in the base of the development area:
```
cd sourcecode
git clone https://github.com/DUNE-DAQ/restcmd.git
cd ..
./build_daq_software.sh --install
```
And now let's start up daq_application:
```
. ./setup_runtime_environment   # Needed to pick up restcmd libraries and applications
daq_application --commandFacility rest://localhost:12345
```
To control it, let's open up a second terminal, set up the environment, and start sending it commands:
```
cd MyTopDir
. ./setup_runtime_environment
python ./sourcecode/restcmd/scripts/send-cmd.py --interactive --file ./sourcecode/restcmd/test/fdpc-commands.json
```
You'll now see
```
Target url: http://localhost:12345/command
This is a list of commands.
Interactive mode. Type the ID of the next command to send, or type 'end' to finish.

Available commands: [u'init', u'conf', u'start', u'stop']
Press enter a command to send next: 
```
And what you want to first is type `"init"`. The surrounding quotes are needed, otherwise you're exited out. You can look in the other terminal running daq_application to see it responding to the commands. Next, type `"conf"` to execute the configuration, and then `"start"` to begin the actual process of moving vectors between the two modules. You should see output like the following:
```
2020-Nov-06 13:22:51,876 DEBUG_0 [dunedaq::appfwk::FakeDataProducerDAQModule::do_work(...) at /home/jcfree/daqbuild_xcheck_instructions/sourcecode/appfwk/test/plugins/FakeDataProducerDAQModule.cpp:118] Produced vector 272 with contents {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8} and size 10 DAQModule: fdp
2020-Nov-06 13:22:52,860 DEBUG_0 [dunedaq::appfwk::FakeDataConsumerDAQModule::do_work(...) at /home/jcfree/daqbuild_xcheck_instructions/sourcecode/appfwk/test/plugins/FakeDataConsumerDAQModule.cpp:122] Received vector 272: {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8} DAQModule: fdc
```
To stop this, send the "stop" command. Ctrl-c will exit you out of these applications. 

<details><summary>daq_application Command Line Arguments</summary>

Use `daq_application --help` to see all of the possible options:
```
bash$ daq_application --help
daq_application known arguments (additional arguments will be stored and passed on):
  -c [ --commandFacility ] arg CommandFacility URI
  -h [ --help ]                produce help message
```

</details>

<details><summary>Some additional information</summary>



### TRACE Messages

To enable the sending of TRACE messages to a memory buffer, you can set one of several TRACE environmental variables _before_ running `appfwk/apps/simple_test_app`.  One example is to use a command like `export TRACE_NAME=TRACE`.  (For more details, please see the [TRACE package documentation](https://cdcvs.fnal.gov/redmine/projects/trace/wiki/Wiki). For example, the [Circular Memory Buffer](https://cdcvs.fnal.gov/redmine/projects/trace/wiki/Circular_Memory_Buffer) section in the TRACE Quick Start talks about the env vars that you can use to enable tracing.)

To view the TRACE messages in the memory buffer, you can use the following additional steps:

* [if not done already] `export SPACK_ROOT=<your spack root> ; source $SPACK_ROOT/setup-env.sh`
* [if not done already] `spack load trace`
* `trace_cntl show` or `trace_cntl show | trace_delta -ct 1` (The latter displays the timestamps in human-readable format.  Note that the messages are listed in reverse chronological order in both cases.)

</details>

## Testing and running inside a docker container

Requirements on the host machine:
* installation of cvmfs (instructions can be found [here](https://wiki.dunescience.org/wiki/DUNE_Computing/Access_files_in_CVMFS#Installing_CVMFS_on_your_computer.28s.29));
* installation of docker (instructions are available on its official website).

Docker image recommended to use is [dingpf/artdaq:latest](https://hub.docker.com/repository/docker/dingpf/artdaq). Its corresponding dockerfile can be found in [this repo](https://github.com/dingp/artdaq-docker).

Use `docker run --rm -it -v /cvmfs:/cvmfs -v $PWD:/scratch dingpf/artdaq` to start the container, where it mounts your local working directory as `/scratch` inside the container. You can then treat `/scratch` as the empty MyTopDir directory referred to in the instructions at the top of this page. 

## Next Step

[[Creating a new package under v1.2.1|Creating-a-new-package-under-v1.2.1]]