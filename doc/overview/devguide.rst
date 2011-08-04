Developing Overview Server
==========================

Install and start the server
----------------------------

Create a development environment as per `HTTP group instructions.
<https://cern.ch/cms-http-group/dev-vm.html>`_ Deploy the ``overview``
server into it::

    cd /data
    A=/data/cfg REPO= VER=mydev PKGS="admin frontend overview"
    $A/Deploy -p $PWD/auth ${REPO} -t $VER -s prep $PWD ${PKGS}
    sudo -H -u _sw bashs -lc "$A/Deploy -p $PWD/auth ${REPO} -t $VER -a -s sw $PWD $PKGS"
    $A/Deploy -p $PWD/auth ${REPO} -t $VER -s post $PWD ${PKGS}
    sudo -H -u _overview bashs -lc "/data/current/config/overview/manage start 'I did read documentation'"
    sudo -H -u _overview bashs -lc "/data/current/config/overview/manage status"

This assumes you have ``/data/auth/overview/DBParam`` with authentication
details for PhEDEx. If you do not have access to PhEDEx database reader
passwords, just create ``/data/auth`` directory but leave it empty. The
server will then be installed with dummy account details; some of the
PhEDEx graphing parts will not work, but the server will nevertheless run.

As usual you can override the version to install or repository parameters
with ``REPO="-r comp=comp.pre"`` and ``PKGS="admin frontend overview@6.0.7"``
for example.

Create development area
-----------------------

Once you have installed the server, create a development area as per `CMS
DM/WM instructions. <https://svnweb.cern.ch/trac/CMSDMWM/wiki/PatchManagement>`_
Clone CMSDMWM/Monitoring from SVN::

    # example dev area, can be anywhere
    mkdir -p /data/user/src
    cd /data/user/src

    # checkout, plus do all other steps from DM/WM instructions
    git svn clone svn+ssh://svn.cern.ch/reps/CMSDMWM/Monitoring -s

Modify code and rebuild
-----------------------

Modify the source code, creating your stg patch stack. For each patch you
can rebuild the server and apply your changes to the server you installed
earlier, restart the server, and test your changes::

    . /data/current/apps/overview/etc/profile.d/env.sh
    monDistPatch -s Overview
    sudo -H -u _overview bashs -lc "/data/current/config/overview/manage start 'I did read documentation'"
    tail -f /data/logs/overview/*.log

Roll back your server
---------------------

If you want to revert your changes and reset the server back to what it was
when installed from RPMs, un-patch and restart it::

    . /data/current/apps/overview/etc/profile.d/env.sh
    monDistUnpatch
    sudo -H -u _overview bashs -lc "/data/current/config/overview/manage start 'I did read documentation'"
    tail -f /data/logs/overview/*.log

Submit your changes
-------------------

Submit your patch or patches in the usual fashion, using CMSDMWM
``submit_patch.py``, creating a trac ticket against Monitoring, or
attaching the changes to an already existing ticket which describes what
is being changed and why. See `the general instructions
<https://svnweb.cern.ch/trac/CMSDMWM/wiki/PatchManagement>`_ for more
information. Once the patch review is completed your changes will be
committed by someone else.

Overview and DQM GUI share the core code. If you are making changes to the
shared code, discuss your changes with appropriate managers *before*
submitting patches to avoid hainv your patch flagged inappropriate. Test
impact on both DQM GUI and Overview, mention you have done so, and note
any adverse effects in the message accompanying your patches.

Patch style notes
-----------------

Submit a series of patches for independent changes. Each patch should make
one logically atomic change. Patches making feature changes should not
include formatting changes, and in particular, should not include pure
white space changes. Make sure ``git diff --check``` does not warn on your
patches, and specifically does not flag trailing white space problems. You
can see the latter in red if you enable git colour diff mode.

Patches are expected to follow a programming style used elsewhere in the
code, and specifically in any surrounding code. This applies to both
python and javascript. Patches which don't follow conventions for use of
white space, naming of things, and general code structuring or layout
automatically get rejected with request to reformat them.
