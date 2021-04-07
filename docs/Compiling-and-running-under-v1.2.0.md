
To get set up, you'll need access to the ups product area `/cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products`, as is the case, e.g., on the lxplus machines at CERN. If you're on a system which has access to this product area, simply do the following after you've logged in to your system and created an empty directory (we'll refer to it as "MyTopDir" on this wiki):
```
curl -O https://raw.githubusercontent.com/DUNE-DAQ/daq-buildtools/v1.2.0/bin/quick-start.sh
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

To work with more repos, add them to the `./sourcecode` subdirectory as we did with appfwk. Be aware, though: if you're developing a new repo which itself depends on another new repo, daq-buildtools may not already know about this dependency; in this case, you'll want to first build and install the second repo, and THEN build and install the first repo. "New" in this context means "not found on https://github.com/DUNE-DAQ as of Oct-15-2020". 

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
_JCF, Oct-16-2020: the set of arguments below no longer works; QueryResponseCommandFacility doesn't exist anymore_
For example, from MyTopDir, you can run 
```bash
daq_application -c QueryResponseCommandFacility -j build/appfwk/test/producer_consumer_dynamic_test.json
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

## Next Step

[[Creating a new package under v1.2.0|Creating-a-new-package-under-v1.2.0]]