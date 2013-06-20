rct-upgrade-microblaze
======================

Xilinx Microblaze Projects for RCT Upgrade 

Setting up the workspace
------------------------

You should only need to do this once.  First, clone the repository:

```shell
cd ~
git clone https://github.com/uwcms/rct-upgrade-microblaze.git
```

Now, open the SDK, and File → Switch Workspace → Other...  Create a workspace
pointing at ``~/rct-upgrade-microblaze``.  Now you need to load all the projects
into the workspace (just once).  Go to File → Import...  Select General →
Existing Projects into Workspace, then hit Next.  Select
``~/rct-upgrade-microblaze`` as the root directory.  Make sure to deselect the
"Copy project sources" option provided in the import wizard, and hit Finish.
Now you should be able to go to Window → Show View → Project Explorer and get
the classic "bunch of junk with tiny tiny editor window" layout with the
imported projects on the left.

More information is available at the [Xilinx git info page.](http://www.xilinx.com/support/documentation/sw_manuals/xilinx14_4/SDK_Doc/reference/sdk_u_cvs.htm)
