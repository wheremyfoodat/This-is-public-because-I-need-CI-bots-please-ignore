[![MacOS Build](https://github.com/wheremyfoodat/This-is-public-because-I-need-CI-bots-please-ignore/actions/workflows/MacOS_Build.yml/badge.svg)](https://github.com/wheremyfoodat/This-is-public-because-I-need-CI-bots-please-ignore/actions/workflows/MacOS_Build.yml) [![Linux Build](https://github.com/wheremyfoodat/This-is-public-because-I-need-CI-bots-please-ignore/actions/workflows/Linux_Build.yml/badge.svg)](https://github.com/wheremyfoodat/This-is-public-because-I-need-CI-bots-please-ignore/actions/workflows/Linux_Build.yml)
# Installation
Clone the repo with `git clone <URL> --init --recursive --remote` to install submodules. Install SFML, then go to the root directory and do
```sh
mkdir build
cd build
cmake .. -G"Unix Makefiles"
make -j2
```

# Credits
@ThePixelGamer - Mental support and saving me countless hours of debugging

![Alt text](/resources/thracia.png?raw=true "Fire Emblem: Thracia 776")