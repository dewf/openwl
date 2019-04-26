import sys
import json

if len(sys.argv) != 2:
    print("usage: gen_bintray.py <build-tag>")
    sys.exit(-1)

build_tag = sys.argv[1]

out_desc = {
    "package": {
        "name": "OpenWL",
        "repo": "TeamGUI",
        "subject": "dewf",
        "vcs_url": "https://github.com/dewf/openwl.git",
        "licenses": ["MIT"],
        # "labels": ["cool", "awesome", "gorilla"],
        "public_download_numbers": True,
        "public_stats": True
    },

    "version": {
        "name": build_tag, #"OpenWL-%s" % (build_tag,),
        "desc": "OpenWL Mac CI Build",
        #"released": "2015-01-04",
        #"vcs_tag": "0.5",
        "gpgSign": False
    },

    "files":
        [
            {
                "includePattern": "OpenWL.dmg", 
                "uploadPattern": "OpenWL/%s/OpenWL-%s-Mac-Framework.dmg" % (build_tag, build_tag),
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


