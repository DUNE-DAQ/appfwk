# Contributing to the Application Framework

During the prototyping stage for the Application Framework, it is recommended to clone the repository and make your changes in a branch. The branch name should be of the form `user/ShortDescription` for clarity in the code browser. This option allows other users to base their changes off of yours, if needed, in a more transparent way than forking.

A code linting tool has been developed by John Freeman that helps *partially* ensure that code is compliant with our style guide, though many potential guideline violations are impossible for a script (as opposed to a human) to catch. Details are below.  

Once your changes are stable, a pull request should be initiated.
Pull requests follow this procedure:
1. The code will be reviewed during an Application Framework meeting
   1.  Comments related to the pull request's intended changes should be incorporated into the branch
   1.  Other comments that may arise should be captured as Github issues
1. The pull request will be allowed to sit for a few days to make sure that there are no further comments
1. Once all comments have been resolved, the Application Framework managers will meet and merge the pull request

## Using the C++ code linter, dune-cpp-style-check.sh

Before code is merged into the develop branch, it should be in conformance with the DUNE C++ Style Guide, available [here](https://github.com/DUNE-DAQ/styleguide/blob/develop/dune-daq-cppguide.md). As mentioned above, a subset of the guidelines are capable of being automatically checked for using a linter tool, `dune-cpp-style-check.sh`. Obtaining `dune-cpp-style-check.sh` is straightforward, as it resides in the same repository, "styleguide", as the actual text of the style guide:
```
git clone https://github.com/DUNE-DAQ/styleguide/
```
You'll want to clone this into the base directory of your development area. In the instructions which follow, we'll call the location of the cloned repo $STYLEGUIDE_DIR (i.e., /my/dev/area/styleguide), and the location of your build output is $BUILD_DIR (i.e., /my/dev/area/build/mypackage_name). First, please read the instructions on how to use the linter, which are printed to output when you run it without arguments:
```
$STYLEGUIDE_DIR/cpplint/dune-cpp-style-check.sh
```
Then, keeping in mind that the most likely location of the JSON compile_commands.json file is in the build directory, to run the linter on a file do the following from the base of your development area:
```
$STYLEGUIDE_DIR/cpplint/dune-cpp-style-check.sh $BUILD_DIR <name of file>
```
...where "file" can be a directory, in which case the script recurses down the directory tree. To get more concrete, if you've got a development area called /my/dev/area and you're in it, and you want to lint all of appfwk, this command would look like:
```
./styleguide/cpplint/dune-cpp-style-check.sh ./build/appfwk appfwk
```
Be aware that the linter simply doesn't have the capability to catch some violations of the guidelines (e.g., figuring out that you've declared data `protected` instead of `private` in a class, or that you've written a function that's meant to do a bunch of unrelated things), and is no substitute for having read and understood the guide. 