FROM cr.yandex/crp18svt3en69ts7nvj1/runtime:22.04-cudagl12-noaudio

USER root

RUN apt-get update && apt-get install -y --fix-missing \
    libgtk2.0-0:i386 \
    libsm6:i386 \
    libnss3 \
    libasound2 \
    libgdk-pixbuf2.0-dev \
    libgtk-3-dev \
    libxss-dev libx11-6 libxcomposite1 libxcursor1 libxdamage1 libxext6 libxfixes3 libxi6 libxrandr2 libxrender1

RUN useradd -ms /bin/bash ue

WORKDIR /app

COPY ./ ./

RUN if [ -f /app/GeoAi/Binaries/Linux/GeoAi-Linux-Shipping ]; then \
        echo "GeoAi-Linux-Shipping" > /app/selected_binary && \
        chmod +x /app/GeoAi/Binaries/Linux/GeoAi-Linux-Shipping && \
        chown ue:ue /app/GeoAi/Binaries/Linux/GeoAi-Linux-Shipping; \
    elif [ -f /app/GeoAi/Binaries/Linux/GeoAi-Linux-DebugGame ]; then \
        echo "GeoAi-Linux-DebugGame" > /app/selected_binary && \
        chmod +x /app/GeoAi/Binaries/Linux/GeoAi-Linux-DebugGame && \
        chown ue:ue /app/GeoAi/Binaries/Linux/GeoAi-Linux-DebugGame; \
    else \
        echo "No valid binary found!" && exit 1; \
    fi

USER ue

CMD /app/GeoAi.sh \
        -PixelStreamingURL=$PixelStreamingURL \
        -RenderOffScreen \
        -AllowPixelStreamingCommands \
        -Unattended \
        -PixelStreamingEncoderRateControl=VBR \
        -PixelStreamingEncoderCodec=H264 \
        -PixelStreamingWebRTCVideoPacingFactor=6 \
        -PixelStreamingWebRTCDisableFrameDropper=true \
        -PixelStreamingWebRTCStartBitrate=5000001 \
        -PixelStreamingEncoderTargetBitrate=9000000 \
        -PixelStreamingWebRTCMinBitrate=5000000 \
        -PixelStreamingWebRTCMaxBitrate=12000000 \
        -PixelStreamingWebRTCFps=60 \
        -PixelStreamingCaptureUseFence=true
