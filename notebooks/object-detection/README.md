# Object Detection

## Requirements
We need to install TAO Toolkit. Please follow instructions 
[here](https://docs.nvidia.com/tao/tao-toolkit/text/tao_toolkit_quick_start_guide.html).

### Virtual environment setup
```bash
conda create --name kitten-detector python=3.9
```

### TAO Toolkit installation

#### Download `getting_started` files
```bash
wget --content-disposition https://api.ngc.nvidia.com/v2/resources/nvidia/tao/tao-getting-started/versions/4.0.0/zip -O getting_started_v4.0.0.zip
unzip -u getting_started_v4.0.0.zip  -d ./getting_started_v4.0.0 && \
rm -rf getting_started_v4.0.0.zip && \
cd ./getting_started_v4.0.0
```

#### Setup pre-requisites

Please do so by following this [instruction](https://docs.nvidia.com/tao/tao-toolkit/text/tao_toolkit_quick_start_guide.html#installing-the-pre-requisites).
Or follow those steps copy-pasted from instruction - you lazy fuck:
```text
The TAO Toolkit launcher is strictly a python3 only package, capable of running on python versions >= 3.6.9.

1. Install docker-ce by following the official instructions.

2. Once you have installed docker-ce, follow the post-installation steps to ensure that the docker can be run without sudo.

3. Install nvidia-container-toolkit by following the install-guide.

4. Get an NGC account and API key:
    4.1. Go to NGC and click the TAO Toolkit container in the Catalog tab. This message is displayed: “Sign in to access the PULL feature of this repository”.
    4.2. Enter your Email address and click Next, or click Create an Account.
    4.3. Choose your organization when prompted for Organization/Team.
    4.4. Click Sign In.

5. Log in to the NGC docker registry (nvcr.io) using the command docker login nvcr.io and enter the following credentials:
    a. Username: "$oauthtoken"
    b. Password: "YOUR_NGC_API_KEY"
where YOUR_NGC_API_KEY corresponds to the key you generated from step 3.
```

#### Install python packages
```bash
bash setup/quickstart_launcher.sh --install
```