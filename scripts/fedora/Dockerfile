FROM fedora:latest

RUN dnf update -y && \
    dnf install -y @development-tools gcc-c++ ninja-build cmake extra-cmake-modules gperf libcurl-devel qt6-qtbase-devel kf6-kcoreaddons-devel kf6-ki18n-devel kf6-knotifications-devel kf6-kxmlgui-devel rpm-build

ARG USER_ID
ARG GROUP_ID

RUN groupadd -g ${GROUP_ID} user && \
    useradd -m -u ${USER_ID} -g ${GROUP_ID} user
USER ${USER_ID}

COPY --chown=${USER_ID}:${GROUP_ID} --chmod=774 build.sh /home/user/build.sh
RUN mkdir /home/user/build && \
    mkdir /home/user/artifacts

WORKDIR /home/user/build
ENTRYPOINT ["/home/user/build.sh"]
