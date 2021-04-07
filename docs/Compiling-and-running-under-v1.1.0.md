To get set up, you'll need access to the ups product area `/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products`, as is the case, e.g., on the lxplus machines at CERN. If you're on a system which has access to this product area, simply do the following after you've logged in to your system and created an empty directory (we'll refer to it as "MyTopDir" on this wiki):
```
curl -O https://raw.githubusercontent.com/DUNE-DAQ/daq-buildtools/v1.1.0/bin/quick-start.sh
sed -i -r 's/edits_check=true/edits_check=false/g' quick-start.sh # Since we're not using quick-start.sh from the head of develop
chmod +x quick-start.sh
./quick-start.sh
```
which will install the needed repos and provide you with a file you can then source to set up the build environment, `setup_build_environment`, and an executable script you can run for builds, `build_daq_software.sh`. Disregard the warning concerning the script's self-check being turned off; the default behavior we've switched off is that quick-start.sh attempts to ensure that it's same as the quick-start.sh at the head of develop in the central repo. 

If, after running `quick-start.sh`, you want to build the v1.1.0 appfwk package and install it in a local subdirectory called `./install` right away, all you need to do is the following:
```
. ./setup_build_environment  # Only needs to be done once in a given shell
./build_daq_software.sh --install
```
`build_daq_software.sh` will by default skip CMake's config+generate stages and go straight to the build stage _unless_ the `CMakeCache.txt` file isn't found in `./build/appfwk` -- which is of course the case the first time you run it. If you want to remove all the contents of `./build/appfwk` and run config+generate+build, all you need to do is add the `--clean` option, i.e.
```
./build_daq_software.sh --clean --install
```
And if, after the build, you want to run the unit tests, just add the `--unittest` option. Note that it can be used with or without `--clean`, so, e.g.:
```
./build_daq_software.sh --clean --install --unittest  # Blow away the contents of ./build/appfwk, run config+generate+build, and then run the unit tests
```
..where in the above case, you blow away the contents of ./build/appfwk,  run config+generate+build, install the result in `./install` and then run the unit tests.

If you want to develop a package other than appfwk, you'll want to add the `--pkgname <package name>` option. E.g., if we want to build listrev (a simple DAQModule package written by Kurt Biery and documented elsewhere on this wiki), from MyTopDir we can do the following:
```
git clone https://github.com/DUNE-DAQ/listrev.git
cd listrev
git checkout v1.1.0
cd ..
./build_daq_software.sh --install --pkgname listrev
```
...where here it's assumed you've already built and locally installed appfwk since it's a dependency of listrev. 

If you want to see verbose output from the compiler, all you need to do is add the `--verbose` option:
```
./build_daq_software.sh --verbose --pkgname <package whose build you're troubleshooting>
```

You can see all the options listed if you run the script with the `--help` command, i.e.
```
./build_daq_software.sh --help
```
Finally, note that both the output of your builds and your unit tests are logged to files in the `./log` subdirectory. These files may have ASCII color codes which make them difficult to read with some tools; `more` or `cat`, however, will display the colors and not the codes themselves. 

</details>

## Running
In order to run the applications built during the above procedure, the system needs to be instructed on where to look for the libraries that can be used to instantiate objects. 
In order to do that, the environmental variable `CET_PLUGIN_PATH` has to contain the path to your compiled `src` and `test` sub-directories. This is handled by the `./setup_runtime_environment` script, which can be found in the daq-buildtools repo which was downloaded: 
```
cp daq-buildtools/scripts/setup_runtime_environment . 
. ./setup_runtime_environment
```
(answer "y" to the `cp: overwrite ‘./setup_runtime_environment’?` prompt)

Note that if you add a new repo to your development area, after building your new code you'll need to source the script again. 

Once the runtime environment is set, just run the application you need.  
For example, from MyTopDir, you can run 
```bash
daq_application -c QueryResponseCommandFacility -j appfwk/test/producer_consumer_dynamic_test.json
```

This example accepts commands _configure_, _start_, _stop_, _unconfigure_, _quit_.

<details><summary>daq_application Command Line Arguments</summary>

Use `daq_application --help` to see all of the possible options:
```
$ ./build/appfwk/apps/daq_application --help
./build/appfwk/apps/daq_application known arguments (additional arguments will be stored and passed on):
  -c [ --commandFacility ] arg CommandFacility plugin name
  -m [ --configManager ] arg   ConfigurationManager plugin name
  -s [ --service ] arg         Service plugin(s) to load
  -j [ --configJson ] arg      JSON Application configuration file name
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

## Next Steps

* [[Creating a new package under v1.1.0|Creating-a-new-package-under-v1.1.0]]
* [[Step-by-step instructions for creating your own DAQModule package under v1.1.0|Step-by-step-instructions-for-creating-your-own-DAQModule-package-under-v1.1.0]]