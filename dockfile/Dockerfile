# Dockerfile for FRS
# base image: dockhub.ghtchina.com:6060/ims/ol_base, openEuler

FROM dockhub.ghtchina.com:6060/ims/arm/ol/base 

# work directory
RUN mkdir -p /app/run/frs/bin
RUN mkdir -p /app/run/frs/libs
RUN mkdir -p /app/run/frs/cfg
RUN mkdir -p /app/run/frs/dat
RUN mkdir -p /app/run/frs/log
RUN mkdir -p /app/run/frs/tmp
RUN chmod 777 /app/run/frs/

WORKDIR /app/run/frs/bin

# libs and app 
COPY ./bin/frssvr /app/run/frs/bin
COPY ./libs/lib*.* /app/run/frs/libs/


# starting to run  FRS
ENTRYPOINT ["./frssvr"]
