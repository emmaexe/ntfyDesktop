FROM ubuntu:rolling

RUN apt update && \
    apt upgrade -y && \
    apt install -y git g++ ninja-build cmake extra-cmake-modules libcurl4-openssl-dev qt6-base-dev libkf6coreaddons-dev libkf6i18n-dev libkf6notifications-dev libkf6xmlgui-dev

ARG USER_ID
ARG GROUP_ID

RUN (deluser --remove-home ubuntu || true) && \
    (delgroup ubuntu || true)
RUN groupadd -g ${GROUP_ID} user && \
    useradd -m -u ${USER_ID} -g ${GROUP_ID} user
USER ${USER_ID}

COPY --chown=${USER_ID}:${GROUP_ID} --chmod=774 build.sh /home/user/build.sh
RUN mkdir /home/user/build && \
    mkdir /home/user/artifacts

WORKDIR /home/user/build
ENTRYPOINT ["/home/user/build.sh"]
