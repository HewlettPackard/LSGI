#!/bin/bash

#“© Copyright 2017  Hewlett Packard Enterprise Development LP

# Redistribution and use in source and binary forms, with or without modification, 
# are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice, 
# this list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without 
# specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.”
#*/

#This script put correct permissions to the files and
#folder incase the checkout has remove them
#It avoids the configuration to fail because of permissions errors
#Author Janneth, Tere
##July
#How to rund
#./setPermissions.sh <LSGI root path>
#./setPermissions.sh /home/user/
#Use
LSGI_PATH=$1

echo ">> Setting permisions to local package"
	#Set permission to scripts *.sh
	cd $LSGI_PATH/LSGi/config; pwd; chmod 755 *.sh
	cd $LSGI_PATH/LSGi/demo/inference; pwd; chmod 755 *.sh
	cd $LSGI_PATH/LSGi/demo/graphQuery; pwd; chmod 755 *.sh
	cd $LSGI_PATH/LSGi/deploy; pwd; chmod 755 *.sh
	cd $LSGI_PATH/LSGi/test/deploy; pwd; chmod 755 *.sh

	#Set permission to executables
	cd $LSGI_PATH/LSGi/demo/inference; pwd; find . -type f -not -name '*.*' -exec chmod 755 \{\} \; 
	cd $LSGI_PATH/LSGi/demo/graphQuery; pwd; find . -type f -not -name '*.*' -exec chmod 755 \{\} \; 

echo "--------------------"
echo "Done with permissions"
