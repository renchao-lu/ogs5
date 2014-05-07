# OGS-5 #

- General homepage: http://www.opengeosys.org
- Wiki: https://svn.ufz.de/ogs
- Build instructions: http://devguide.opengeosys.org

## Quickstart ##

	cd [source-directory]
	mkdir build
	cd build
	cmake .. -DOGS_FEM=ON

Open the Visual Studio solution which was created in the build-directory or just type `make` on Linux.

# Using Git and GitHub #

Following ["A successful Git branching model"](http://nvie.com/posts/a-successful-git-branching-model/), we suggest to have the following two main branches
- ***master*** - the main branch where the source code of HEAD always reflects the latest official codes (i.e. trunk in SVN repository)
- ***development*** - the main branch where the source code of HEAD always reflects a state with the latest delivered development changes for the next release.

To implement new features, every developer
1. forks this repository to have their own (your repository is still private)
2. makes a branch from master or development branch, and implement their stuff
3. pushes the local branch to its GitHub repository
4. makes a pull request from its GitHub repository to development branch in the `envinf/ogs5-trunk` repository

Once the development branch is ready to release a new version (this can also be done by one of the OGS core developers)
1. merge the development branch into master
2. push all new commits in master to the SVN repository using `git svn dcommit`

## Enable the Git to Subversion connection ##

To be able to sync from and to the Subversion trunk you have to do the following steps on your local repository:

    git svn init https://svn.ufz.de/svn/ogs/trunk/sources
    git svn fetch
    git svn rebase

Now you sync the current branch (use **always** the **master** branch!) to Subversion with `git svn dcommit` (you may also want to append `--dry-run` to the command to see what gets synced before you actually sync).
