## Overview

The Edgegap Unreal Plugin is designed to streamline your game server deployment process, allowing you to focus on what really matters: your game. With minimal configuration, you can have your game server up and running on a remote, deployed server within minutes.

## Features

1. **Create an app on our platform:** No need to manually set up an app; the plugin does it for you.
2. **Package your game server for Linux:** Automatically packages your game server in a Linux-compatible format.
3. **Build the container:** Constructs a Docker container for your game server.
4. **Push the game server container to your private registry:** Your game server container is securely pushed to your private registry.
5. **Create a new app version for the pushed container:** A new version of your app is created automatically.

This is an excellent alternative to traditional CICD pipelines, making your server development process faster and more straightforward.

## Prerequisites

Before you get started, make sure you have the following prerequisites in place:

- **Unreal Cross-Compiling Toolchain:** Required for packaging your game server for Linux. [Follow the instructions here.](https://docs.unrealengine.com/5.0/en-US/linux-development-requirements-for-unreal-engine/)
- **Docker:** [Install Docker](https://docs.docker.com/desktop/install/windows-install/) on your machine. For more details, refer to our [Docker documentation](https://docs.edgegap.com/docs/category/container).
- **API Token:** You must have an API token. If you don't have one, [see this page.](https://docs.edgegap.com/docs/getting-started)
- **Container Repository Access:** You'll need access to our container repository. [More information can be found here.](https://docs.edgegap.com/docs/container/edgegap-container-registry)

## Installation

1. **Download**: Download the plugin compatible with your Unreal Engine version.
2. **Installation**: Copy the `Edgegap` folder to your project's `Plugins` folder.
3. **Enable Plugin**: Open `Edit > Plugins` in the Unreal Editor and enable the Edgegap plugin.

![Installation1](https://docs.edgegap.com/assets/images/plugins_menu-0666ddd36d0767891a1e9c3ac5ce77a7.png)
![Installation2](https://docs.edgegap.com/assets/images/enable_plugin-482110ad3c2fdb6546e5fee670a09db5.png)

## Configuration

Navigate to `Edit > Project Settings` and scroll down to the `Plugins` category to find the `Edgegap` section.

![Configuration1](https://docs.edgegap.com/assets/images/project_settings_menu-55ee5aca91fb13225f04c99495355f4b.png)
![Configuration2](https://docs.edgegap.com/assets/images/plugin_configuration-854a1ba57dfa27541e4cb3cbf0d57635.png)

### General Information

| Field     | Description                                |
|-----------|--------------------------------------------|
| API Token | The API token to use for the plugin.       |

### Application Info

| Field            | Description                                                |
|------------------|------------------------------------------------------------|
| Application Name | The name for your game server application within the platform. |
| Image Path       | Select an image for your game server app.                   |

Click the "Create Application" button to finalize the app creation.

### Versioning

| Field                  | Description                                             |
|------------------------|---------------------------------------------------------|
| Version Name           | Name your application version.                           |
| Registry               | Host of the container registry.                          |
| Image Repository       | Repository to use within the provided registry.          |
| Tag                    | Image tag for your container.                            |
| Private Registry Username | Username for your private registry.                    |
| Private Registry Token | Password for your private registry.                      |

## Standard Workflow

1. Complete the initial configuration and create your application.
2. Set up a working game server.
3. Use the "Create Version" button to package and push your game server container.
4. Deploy an instance of your game server using the "Deploy Created Version" button.
5. Connect to the deployed game server using the host and port displayed in the plugin window.

## Current Deployments

This section displays your current deployments on our platform. Use the "Deploy Created Version" and "Refresh" buttons to manage your deployments.

![Current Deployments](https://docs.edgegap.com/assets/images/running_deployment-7de51237f43c45a51b93d797ecf2a7a4.png)

---

Thank you for choosing Edgegap's Unreal Plugin! 🎮
