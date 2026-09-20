{
  stdenv,
  fetchFromGitHub,
  fetchurl,
  runCommandWith,
  lib,
  cmake,
  ninja,
  gperf,
  kdePackages,
  qt6,
  curl,
  nlohmann_json,
}: let
  cpmDownloadVersion = "0.42.0";
  cpmSrc = fetchurl {
    url = "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${cpmDownloadVersion}/CPM.cmake";
    hash = "sha256-ICC0/ELbpEgXmD4GNC5oLs/D0vSEpYHxHMVzH75Nzoo=";
  };
  cpmSourceCache =
    runCommandWith {
      name = "cpm-source-cache";
      runLocal = true;
    }
    ''
      mkdir -p $out/cpm
      ln -s ${cpmSrc} $out/cpm/CPM_${cpmDownloadVersion}.cmake
    '';

  cpmSrcs = [
    (fetchFromGitHub {
      name = "emojicpp";
      owner = "emmaexe";
      repo = "emojicpp";
      rev = "v3.0.0";
      hash = "sha256-2jFHRJvD/YQTFvjStGVm6ea/7+Tb3U9TF/JDlmFe0zY=";
    })
  ];

  rootDir = ../..;
in
  stdenv.mkDerivation {
    name = "ntfyDesktop";

    src = rootDir;

    nativeBuildInputs = [
      cmake
      ninja
      gperf
      kdePackages.extra-cmake-modules
      qt6.wrapQtAppsHook
    ];

    buildInputs = [
      curl
      qt6.qtbase
      kdePackages.kcoreaddons
      kdePackages.ki18n
      kdePackages.knotifications
      kdePackages.kxmlgui

      nlohmann_json
    ];

    # dontUnpack = false;

    postUnpack = (
      lib.strings.concatLines (
        lib.lists.forEach cpmSrcs (
          s:
          # Make CPM sources writable for patches and set CPM_<package>_SOURCE flags
          ''
            cp -R ${s.out} ${s.name}
            chmod -R u+w ${s.name}
            appendToVar cmakeFlags -DCPM_${s.name}_SOURCE=$(pwd)/${s.name}
          ''
        )
      )
    );

    dontPatch = true;
    # dontConfigure = false;

    cmakeFlags = [
      (lib.cmakeFeature "CPM_SOURCE_CACHE" "${cpmSourceCache}")
      (lib.cmakeBool "CPM_LOCAL_PACKAGES_ONLY" true)
      # Used so that `find_package` can locate some dependencies with `Find*.cmake` files
      (lib.cmakeFeature "CMAKE_MODULE_PATH" "${./cmake}")
    ];

    # dontBuild = false;
    # doCheck = false;
    # dontInstall = false;
    # dontFixup = false;
    # doInstallCheck = false;
  }
