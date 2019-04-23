import sys
import json

if len(sys.argv) != 2:
    print("usage: gen_bintray.py <build-ID>")
    sys.exit(-1)

build_id = sys.argv[1]

out_desc = {
    "package": {
        "name": "MyPackage",
        "repo": "OpenWL",
        "subject": "dewf",
        "vcs_url": "https://github.com/dewf/openwl.git",
        "licenses": ["MIT"],
        "labels": ["cool", "awesome", "gorilla"],
        "public_download_numbers": True,
        "public_stats": True
    },

    "version": {
        "name": "OpenWL-%s" % (build_id,),
        "desc": "OpenWL Mac CI Build",
        #"released": "2015-01-04",
        #"vcs_tag": "0.5",
        "gpgSign": False
    },

    "files":
        [
            {
                "includePattern": "OpenWL.dmg", 
                "uploadPattern": "travis-builds/%06d/OpenWL.dmg" % (int(build_id),),
                "matrixParams":
                { 
                    "override": 1 
                }
            }
        ],
    "publish": True
}
with open("bintray.json", "w") as outfile:
    json.dump(out_desc, outfile, indent=4)


