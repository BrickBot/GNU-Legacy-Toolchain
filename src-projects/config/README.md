Standardized Files
==================
This project folder contains standardized files that have been centralized for
consistency, uniformity, and ease of maintenance across all projects.

The source of `config.guess` and `config.sub` is the
[savannah.gnu.org Git repository for Config](https://cgit.git.savannah.gnu.org/cgit/config.git/tree/).
These files can be updated by running the `update-config.sh` shell script
located in this folder.

The source of `ltconfig` originates from GCC 4.4.7’s libjava/classpath project.
As the most recent version across all such sources within this repository,
this file was selected as the basis for standardization.
