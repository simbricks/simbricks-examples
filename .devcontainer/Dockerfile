FROM simbricks/simbricks
ENV TZ=Europe/Berlin
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update \
 && apt-get install -y \
	git \
  python3-graphviz \
	python3-pip \
  python3-ipykernel \
 && rm -rf /var/lib/apt/lists/*
