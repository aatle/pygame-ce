#!/usr/bin/env python
"""pygame.examples.pixelarray

PixelArray does array processing of pixels.
Sort of like another array processor called 'numpy' - But for pixels.

    Flip it,
            stripe it,
                      rotate it.

Controls
--------

To see different effects - press a key or click a mouse.
"""

import os

import pygame

main_dir = os.path.split(os.path.abspath(__file__))[0]
data_dir = os.path.join(main_dir, "data")


def show(image):
    screen = pygame.display.get_surface()
    screen.fill((255, 255, 255))
    screen.blit(image, (0, 0))
    pygame.display.flip()
    while True:
        event = pygame.event.wait()
        if event.type == pygame.QUIT:
            pygame.quit()
            raise SystemExit
        if event.type in [pygame.MOUSEBUTTONDOWN, pygame.KEYDOWN]:
            break


def main():
    pygame.init()

    pygame.display.set_mode((255, 255))
    surface = pygame.Surface((255, 255))

    pygame.display.flip()

    # Create the PixelArray.
    ar = pygame.PixelArray(surface)

    # Do some easy gradient effect.
    for y in range(255):
        r, g, b = y, y, y
        ar[:, y] = (r, g, b)
    del ar
    show(surface)

    # We have made some gradient effect, now flip it.
    ar = pygame.PixelArray(surface)
    ar[:] = ar[:, ::-1]
    del ar
    show(surface)

    # Every second column will be made blue
    ar = pygame.PixelArray(surface)
    ar[::2] = (0, 0, 255)
    del ar
    show(surface)

    # Every second row will be made green
    ar = pygame.PixelArray(surface)
    ar[:, ::2] = (0, 255, 0)
    del ar
    show(surface)

    # Manipulate the image. Flip it around the y axis.
    surface = pygame.image.load(os.path.join(data_dir, "arraydemo.webp"))
    ar = pygame.PixelArray(surface)
    ar[:] = ar[:, ::-1]
    del ar
    show(surface)

    # Flip the image around the x axis.
    ar = pygame.PixelArray(surface)
    ar[:] = ar[::-1, :]
    del ar
    show(surface)

    # Every second column will be made white.
    ar = pygame.PixelArray(surface)
    ar[::2] = (255, 255, 255)
    del ar
    show(surface)

    # Flip the image around both axes, restoring its original layout.
    ar = pygame.PixelArray(surface)
    ar[:] = ar[::-1, ::-1]
    del ar
    show(surface)

    # Rotate 90 degrees clockwise.
    w, h = surface.get_size()
    surface2 = pygame.Surface((h, w), surface.get_flags(), surface)
    ar = pygame.PixelArray(surface)
    ar2 = pygame.PixelArray(surface2)
    ar2[...] = ar.transpose()[::-1, :]
    del ar, ar2
    show(surface2)

    # Scale it by throwing each second pixel away.
    surface = pygame.image.load(os.path.join(data_dir, "arraydemo.webp"))
    ar = pygame.PixelArray(surface)
    sf2 = ar[::2, ::2].make_surface()
    del ar
    show(sf2)

    # Replace anything looking like the blue color from the text.
    ar = pygame.PixelArray(surface)
    ar.replace((60, 60, 255), (0, 255, 0), 0.06)
    del ar
    show(surface)

    # Extract anything which might be somewhat black.
    surface = pygame.image.load(os.path.join(data_dir, "arraydemo.webp"))
    ar = pygame.PixelArray(surface)
    ar2 = ar.extract((0, 0, 0), 0.07)
    sf2 = ar2.surface
    del ar, ar2
    show(sf2)

    # Compare two images.
    surface = pygame.image.load(os.path.join(data_dir, "alien1.gif"))
    surface2 = pygame.image.load(os.path.join(data_dir, "alien2.gif"))
    ar1 = pygame.PixelArray(surface)
    ar2 = pygame.PixelArray(surface2)
    ar3 = ar1.compare(ar2, 0.07)
    sf3 = ar3.surface
    del ar1, ar2, ar3
    show(sf3)


if __name__ == "__main__":
    main()
    pygame.quit()
