FROM mlikiowa/napcat-docker:base

WORKDIR /usr/src/app

COPY SignerServer-ubuntu-latest-x64.zip .

# 安装 Linux QQ
RUN arch=$(arch | sed s/aarch64/arm64/ | sed s/x86_64/amd64/) && \
    curl -o linuxqq.deb https://dldir1.qq.com/qqfile/qq/QQNT/8b4fdf81/linuxqq_3.2.7-23361_${arch}.deb && \
    dpkg -i --force-depends linuxqq.deb && rm linuxqq.deb

# 安装 SignerServer
RUN arch=$(arch | sed s/aarch64/arm64/ | sed s/x86_64/x64/) && \
    unzip SignerServer-ubuntu-latest-${arch}.zip && \
    chrom +x start.sh

# 配置 supervisord
RUN echo "[supervisord]" > /etc/supervisord.conf && \
    echo "nodaemon=true" >> /etc/supervisord.conf && \
    echo "[program:napcat]" >> /etc/supervisord.conf && \
    echo "command=COMMAND" >> /etc/supervisord.conf

ENTRYPOINT ["sh", "start.sh"]

