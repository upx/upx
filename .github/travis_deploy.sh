#! /bin/bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Support for Travis CI -- https://travis-ci.org/upx/upx/builds
# Copyright (C) Markus Franz Xaver Johannes Oberhumer

if [[ $TRAVIS_OS_NAME == osx ]]; then
argv0=$0; argv0abs=$(greadlink -en -- "$0"); argv0dir=$(dirname "$argv0abs")
else
argv0=$0; argv0abs=$(readlink -en -- "$0"); argv0dir=$(dirname "$argv0abs")
fi
source "$argv0dir/travis_init.sh" || exit 1

set -x # debug

if [[ $BM_X == rebuild-stubs ]]; then exit 0; fi
# save space
if [[ $BM_B =~ (^|\+)coverage($|\+) ]]; then exit 0; fi
if [[ $BM_B =~ (^|\+)debug($|\+) ]]; then exit 0; fi
if [[ $BM_B =~ (^|\+)sanitize($|\+) ]]; then exit 0; fi
if [[ $BM_B =~ (^|\+)scan-build($|\+) ]]; then exit 0; fi
if [[ $BM_B =~ (^|\+)valgrind($|\+) ]]; then exit 0; fi

# prepare $subdir and $branch
subdir=
branch=
if [[ -n $APPVEYOR_JOB_ID ]]; then
    branch=$APPVEYOR_REPO_BRANCH
    if [[ -n $APPVEYOR_PULL_REQUEST_NUMBER ]]; then exit 0; fi
elif [[ -n $GITLAB_CI ]]; then
    subdir=$CI_BUILD_NAME
    branch=$CI_BUILD_REF_NAME
else
    branch=$TRAVIS_BRANCH
    if [[ "$TRAVIS_PULL_REQUEST" != "false" ]]; then exit 0; fi
fi
subdir=${subdir%%:*}; subdir=${subdir%%/*}; subdir=${subdir%%--*}
case $branch in
    devel*) ;;
    gitlab*) ;;
    master*) ;;
    travis*) ;;
    *) exit 0;;
esac

# get $rev, $branch and $git_user
cd / && cd $upx_SRCDIR || exit 1
rev=$(git rev-parse --verify HEAD)
if [[ "$branch" == gitlab* ]]; then
    if git fetch -v origin devel; then
        git branch -a -v
        rev=$(git merge-base origin/devel $rev || echo $rev)
    fi
fi
timestamp=$(git log -n1 --format='%at' $rev)
date=$(TZ=UTC0 date -d "@$timestamp" '+%Y%m%d-%H%M%S')
#branch="$branch-$date-${rev:0:6}"
# make the branch name a little bit shorter so that it looks nice on GitHub
branch="$branch-${date:0:8}-${rev:0:7}"
if [[ -n $APPVEYOR_JOB_ID ]]; then
    branch="$branch-appveyor"
    git_user="AppVeyor CI"
elif [[ -n $GITLAB_CI ]]; then
    branch="$branch-gitlab"
    git_user="GitLab CI"
else
    branch="$branch-travis"
    git_user="Travis CI"
fi
unset timestamp date

# /***********************************************************************
# // prepare directory $d
# ************************************************************************/

cd / && cd $upx_BUILDDIR || exit 1

mkdir deploy || exit 1
chmod 700 deploy
cd deploy || exit 1

if [[ -n $BM_CROSS ]]; then
    d=$BM_CROSS
else
    cpu=unknown
    case $BM_C in
        clang*-m32  | gcc*-m32)  cpu=i386;;
        clang*-m64  | gcc*-m64)  cpu=amd64;;
        clang*-mx32 | gcc*-mx32) cpu=amd64;;
        msvc*-x86) cpu=i386;;
        msvc*-x64) cpu=amd64;;
    esac
    os=$TRAVIS_OS_NAME
    if [[ $os == osx ]]; then
        os=darwin
    elif [[ $os == windows ]]; then
        [[ $cpu == i386 ]] && os=win32
        [[ $cpu == amd64 ]] && os=win64
    fi
    d=$cpu-$os
fi
d=$d-$BM_C-$BM_B

if [[ -n $subdir ]]; then
    print_header "DEPLOY $subdir/$d"
else
    print_header "DEPLOY $d"
fi

mkdir $d || exit 1
for exeext in .exe .out; do
    f=$upx_BUILDDIR/upx$exeext
    if [[ -f $f ]]; then
        cp -p -i $f  $d/upx-git-${rev:0:12}$exeext
        sha256sum -b $d/upx-git-${rev:0:12}$exeext
    fi
done

# /***********************************************************************
# // clone, add files & push
# ************************************************************************/

new_branch=0
if ! git clone -b "$branch" --single-branch https://github.com/upx/upx-automatic-builds.git; then
    git  clone -b master    --single-branch https://github.com/upx/upx-automatic-builds.git
    new_branch=1
fi
cd upx-automatic-builds || exit 1
chmod 700 .git
git config user.name "$git_user"
git config user.email "none@none"
if [[ $new_branch == 1 ]]; then
    git checkout --orphan "$branch"
    git reset --hard || true
fi
git ls-files -v

if [[ -n $subdir ]]; then
    [[ -d $subdir ]] || mkdir $subdir
    mv ../$d $subdir/
    git add $subdir/$d
else
    mv ../$d .
    git add $d
fi

if git diff --cached --exit-code --quiet >/dev/null; then
    # nothing to do ???
    exit 0
fi

now=$(date '+%s')
##date=$(TZ=UTC0 date -d "@$now" '+%Y-%m-%d %H:%M:%S')
git commit --date="$now" -m "Automatic build $d"
git ls-files -v
#git log --pretty=fuller

umask 077
[[ -d ~/.ssh ]] || mkdir ~/.ssh
fix_home_ssh_perms
repo=$(git config remote.origin.url)
ssh_repo=${repo/https:\/\/github.com\//git@github.com:}
eval $(ssh-agent -s)
set +x # IMPORTANT
openssl aes-256-cbc -d -a -K "$UPX_AUTOMATIC_BUILDS_SSL_KEY" -iv "$UPX_AUTOMATIC_BUILDS_SSL_IV" -in "$upx_SRCDIR/.github/upx-automatic-builds@github.com.enc" -out .git/deploy.key
set -x
chmod 600 .git/deploy.key
ssh-add .git/deploy.key
fix_home_ssh_perms
ssh-keyscan -H github.com >> ~/.ssh/known_hosts
fix_home_ssh_perms

let i=0 || true
while true; do
    if [[ $i -ge 10 ]]; then
        echo "ERROR: git push failed"
        exit 1
    fi
    if [[ $new_branch == 1 ]]; then
        if git push -u $ssh_repo "$branch"; then break; fi
    else
        if git push    $ssh_repo "$branch"; then break; fi
    fi
    git branch -a -v
    git fetch -v origin "$branch"
    git branch -a -v
    git rebase FETCH_HEAD
    git branch -a -v
    sleep $((RANDOM % 5 + 1))
    let i+=1
done

exit 0
