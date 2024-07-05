# DAQ Application

`daq_application` is the main entry point for all _dunedaq_ processes that contain DAQModules and Queues. At program start, every `daq_application` instance only knows the basic information necessary for it to connect to Run Control and receive more information via the `init` and `conf` commands.

# Command Line

```
daq_application --help
daq_application known arguments (additional arguments will be stored and passed on):
  -n [ --name ] arg                     Application name
  -p [ --partition ] arg (=global)      Partition name
  -c [ --commandFacility ] arg          CommandFacility URI
  -d [ --confFacility ] arg             ConfFacility URI
  -i [ --informationService ] arg (=stdout://flat)
                                        Information Service URI
  -h [ --help ]                         produce help message
```

`daq_application` has three required arguments: `--name`, which sets the application name for use in operational monitoring and Run Control, `--commandFacility`, a URI that is used to load the appropraite CommandFacility plugin and connect to Run Control (e.g. `stdin://test-job.json` loads the STDIN Command Facility plugin and reads the test-job.json job description file), and `--confFacility`, a URI that is used to load the ConfFacility plugin to retrieve configuration data (e.g. file://dir_containing_files_of_type_app_name_command.json).

`--informationService` is used to set the URI for operational monitoring output; by default OpMon will be logged to stdout.

`--partition` will be used in the future to help identify `daq_application` instances which belong to different Run Control units and are independent of one another

# Usage Notes

As of v2.6.0, `daq_application` will seldom have to be called directly, instead the preferred method of starting _dunedaq_ applications will be to use one of the Run Control products, such as `nanorc` or `drunc`.
