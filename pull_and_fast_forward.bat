@echo off

:: grab latest commits from server
echo
echo '******************************************'
echo '*** pulling latest commits from server ***'
echo '******************************************'
echo
git pull --rebase --recurse-submodules

:: if only submodules shall be updated, use the next line instead of the command above
:: git submodule update --recursive --remote

:: above command will set current branch to detached HEAD. set back to master.
echo
echo '******************************************************'
echo '*** checking out master branches in all submodules ***'
echo '******************************************************'
echo
git submodule foreach git checkout master

:: now do pull to fast-forward to latest commit
echo
echo '**************************************'
echo '*** fast-forwarding all submodules ***'
echo '**************************************'
echo
git submodule foreach git pull origin master
