#!/bin/bash -x

set +e

## get MII version information for artifact and tag name
MII_MAJOR_VERSION=$(grep ^MI_MAJOR_VERSION version | cut -f2 -d"=")
MII_MINOR_VERSION=$(grep ^MI_MINOR_VERSION version | cut -f2 -d"=")
MII_PATCH_LEVEL=$(grep ^MI_PATCH_LEVEL version | cut -f2 -d"=")

## get GAMS version information for artifact, tag name and description
export GAMS_DISTRIB_MAJOR_VERSION=$(grep ^GAMS_DISTRIB_MAJOR version | cut -f2 -d"=")

## get changelog content
CHANGELOG=""
foundFirst="false"
while IFS="" read -r line
do
    if [[ $line =~ ^Version.* ]] && [[ $foundFirst == "true" ]]; then
        break
    elif [[ $line =~ ^Version.* ]]; then
        foundFirst="true"
    elif [[ $line =~ ^=.* ]]; then
    	 continue
    else
        CHANGELOG+=$line$'\n'
    fi
done < CHANGELOG

## GitHub parameters
GITHUB_ORGA=GAMS-dev
GITHUB_REPO=model-inspector

## delete old release if needed
RELEASE_DELETED=0
github-release delete --user $GITHUB_ORGA --repo $GITHUB_REPO --tag ${TAG_NAME} && RELEASE_DELETED=0 || RELEASE_DELETED=1
if [[ $RELEASE_DELETED -eq 0 ]]; then
  echo "Release deleted"
else
  echo "No release to delete"
fi

## create a GitHub release
if [[ ${TAG_NAME} =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    github-release release --user $GITHUB_ORGA --repo $GITHUB_REPO --tag ${TAG_NAME} \
--description "${CHANGELOG}

This version of GAMS Model Instance Inspector requires GAMS ${GAMS_DISTRIB_MAJOR_VERSION}. To download GAMS, please visit https://www.gams.com/latest/. To learn more about GAMS Model Instance Inspector, please visit https://www.gams.com/latest/docs/T_MAIN.html"
elif [[ ${TAG_NAME} =~ ^v[0-9]+\.[0-9]+\.[0-9]+-rc\.?[0-9]*$ ]]; then
    github-release release --user $GITHUB_ORGA --repo $GITHUB_REPO --tag ${TAG_NAME} --pre-release \
--description "${CHANGELOG}

This version of GAMS Model Instance Inspector requires GAMS ${GAMS_DISTRIB_MAJOR_VERSION}. To download GAMS, please visit https://www.gams.com/latest/. To learn more about GAMS Model Instance Inspector, please visit https://www.gams.com/latest/docs/T_MAIN.html"
fi

## upload artifacts to GitHub
if [[ ${TAG_NAME} =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    cd artifacts
    mv $(find *arm_64.dmg) "GAMS_MII-$MII_MAJOR_VERSION.$MII_MINOR_VERSION.$MII_PATCH_LEVEL-arm_64.dmg"
    mv $(find *x86_64.dmg) "GAMS_MII-$MII_MAJOR_VERSION.$MII_MINOR_VERSION.$MII_PATCH_LEVEL-x86_64.dmg"
    mv $(find *AppImage) "GAMS_MII-$MII_MAJOR_VERSION.$MII_MINOR_VERSION.$MII_PATCH_LEVEL-x86_64.AppImage"
    mv $(find *zip) "GAMS_MII-$MII_MAJOR_VERSION.$MII_MINOR_VERSION.$MII_PATCH_LEVEL-x86_64.zip"
    sleep 16
    parallel github-release --verbose upload -R --user $GITHUB_ORGA --repo $GITHUB_REPO --tag ${TAG_NAME} --name {} --file {} ::: *.*
fi
