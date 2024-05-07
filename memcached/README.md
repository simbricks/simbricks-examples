# Memcached Example

In this example we want to run the SimBricks experiment located under 
`experiment/memcached-experiment.py`. The given example makes use of 
[memcached](https://memcached.org/) which can not be used with the base image 
provided by [SimBricks](https://simbricks.github.io/). Therefore, in this example 
we want to show how a user can provide an image in which memcached is available 
that can be used alongside SimBricks.

A simple way of creating such an image is by making use of [packer](https://www.packer.io/).
Inside `Makefile` you can find an example on how to use and 
start packer. The Makefile upon start first downloads packer and makes it available 
for usage (see packer target). In a next step the `Makefile` (IMAGES target) makes 
use of packer to build an actual image in which memcached is installed.

Packer requires that a user provides a configuration file as for 
example `image/image.pkr.hcl`. In the given packer configuration we make use of 
the SimBricks base image (`/simbricks/images/output-base/base`) that is already 
provided by the [devcontainer](https://github.com/simbricks/simbricks-examples/blob/main/.devcontainer.json)
given in this example repository. If a user now wants to use memcached 
or any other program, packer can be used to install the required dependecies inside 
the SimBricks base image to create a new image. To do so, a user must provide 
an installation script which handels the actual installation and setup of the required 
software, in this case memcached as shown in `image/install.sh`. Packer is then 
pointed to this installation script as shown in the packer configuration file 
[here](https://github.com/simbricks/simbricks-examples/blob/939dbd8e98304631ea6566622fa878256ee8af32/memcache-cluster/image/image.pkr.hcl#L57).
The same way users can also provide a cleanup script. An example for such a script 
can be found under `image/cleanup.sh`.

The given `Makefile` invokes the SimBricks experiment rigth after the required 
image was built. The built image will be put into a folder called `output-memcached`.
To instruct the SimBricks simulation to actually use the out-of-tree built image,
a user can simply set the `disk_image` property of the involved host simulators 
[node config](https://simbricks.readthedocs.io/en/latest/user/orchestration.html)
to an **absolute** path pointing to the actual image file located under `output-memcached`.
An example can be seen in the given `experiment/memcached-experiment.py` SimBricks 
simulation script [here](https://github.com/simbricks/simbricks-examples/blob/f7d3500bc1c2d32bc4eb62a8b22637ba39797921/memcached/experiment/memcached-experiment.py#L55C1-L55C95).

