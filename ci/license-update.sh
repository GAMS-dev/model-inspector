#!/bin/bash

## Setup MII repository
git clone https://${GITLAB_USER}:${GITLAB_CI_UPDATE_TOKEN}@git.gams.com/devel/gams-mii.git mii
cd mii
git config user.name ${GITLAB_USER}
git config user.email ${GITLAB_USER_EMAIL}
git checkout $CI_COMMIT_REF_NAME
cd ..

## Update copyright and license
git clone https://${GITLAB_USER}:${GITLAB_CI_UPDATE_TOKEN}@git.gams.com/afust/license-update-script.git script
./script/src/update-license.py -p $CI_PROJECT_DIR/mii/src --mii-license

## Commit and push updates
cd mii
CHANGES=$(git diff --name-only)
if [[ "$CHANGES" == "" ]]; then
    echo "NO CHANGES... NOTHING TO PUSH"
else
    echo "update copyright, license"
    git add -u
    git commit -m "pipeline update copyright, license"
    # prevent ci run via -o ci.skip
    git push -o ci.skip
fi