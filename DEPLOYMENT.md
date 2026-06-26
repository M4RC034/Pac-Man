# Deployment

This project is an Allegro desktop game. To run it online in a browser, the Docker image starts a virtual X display and exposes it through noVNC on port `7860`.

## Local Docker

```bash
docker build -t pacman-allegro .
docker run --rm -p 7860:7860 pacman-allegro
```

Open `http://localhost:7860/vnc.html?autoconnect=true&resize=scale`.

## Hugging Face Spaces

1. Create a new Space.
2. Choose **Docker** as the SDK.
3. Push this repository to the Space.
4. Hugging Face will build the `Dockerfile` and expose port `7860`.

The app URL can use:

```text
/vnc.html?autoconnect=true&resize=scale
```

## GitHub

GitHub Pages cannot run this native Allegro app directly. Good GitHub options are:

- Host the source repository and Dockerfile.
- Build and publish the Docker image with GitHub Actions to GitHub Container Registry.
- Deploy the published image to a runtime that supports containers, such as Hugging Face Spaces, Render, Fly.io, Railway, or a VM.

## Runtime Notes

- The container uses `ALSOFT_DRIVERS=null` so Allegro/OpenAL can initialize audio without a host sound device.
- Some gameplay TODOs from the original hackathon template are still present. The Docker image can build and launch, but not every game feature may be implemented yet.
