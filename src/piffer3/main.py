import pygame
import os

pygame.init()

MARGIN = 10

CANVAS_SIZE = (10, 10)
canvas = pygame.Surface(CANVAS_SIZE)

adapt_tiles = ([pygame.Surface(CANVAS_SIZE) for _ in range(4)]
               + [canvas]
               + [pygame.Surface(CANVAS_SIZE) for _ in range(11)]
               + [pygame.Surface(CANVAS_SIZE, pygame.SRCALPHA) for _ in range(4)])
# adapt_indices = [0b0110, 0b0010, 0b0011,
#                  0b0100, 0b1111, 0b0001,
#                  0b1100, 0b1000, 0b1001]

preview_scale = 2

color = (0, 0, 255)
secondary = (0, 0, 255)

palette = [(0, 0, 0), (0, 0, 255), (0, 255, 0), (0, 255, 255),
           (255, 0, 0), (255, 0, 255), (255, 255, 0), (255, 255, 255)]

working_dir = '.'
dir_list = sorted(os.listdir(working_dir))
dir_i = 0

choose_file_file_name = ''

tile_modes = ['', 'tile', 'adapt']
tile_mode = ''

zoom_timer = 1000

state = 'draw'
state_stack = [state]
data_stack = []
clock = pygame.time.Clock()
font_size = 100
font = pygame.font.SysFont('ubuntu mono', font_size)
screen = pygame.display.set_mode((0, 0), pygame.RESIZABLE)
size = screen.get_size()
run = True
while run:
    clock.tick(60)

    if state == 'draw':
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False
            elif event.type == pygame.VIDEORESIZE:
                size = event.size

                font_size = size[1] // 10
                font = pygame.font.SysFont('ubuntu mono', font_size)
                # print('resize', size)
            elif event.type == pygame.KEYUP:
                if event.mod & pygame.KMOD_CTRL:
                    if event.key == pygame.K_p:
                        state_stack.append(state)
                        state = 'ctrl_p'
                    elif event.key == pygame.K_t:
                        state_stack.append(state)
                        state = 'ctrl_t'
                    elif event.key == pygame.K_s:
                        if event.mod & pygame.KMOD_SHIFT:
                            state_stack.append(state)
                            state = 'ctrl_shift_s'
                        else:
                            state_stack.append(state)
                            state = 'ctrl_s'

                    elif event.key == pygame.K_f:
                        canvas.fill(color)

                    elif event.key == pygame.K_MINUS:
                        zoom_timer = 0
                        preview_scale //= 2
                        if preview_scale == 0:
                            preview_scale = 1
                    elif event.key == pygame.K_EQUALS:
                        zoom_timer = 0
                        preview_scale *= 2

        screen.fill((50, 50, 50))

        s = min(size) // CANVAS_SIZE[0] * CANVAS_SIZE[0]
        pixel_size = s // CANVAS_SIZE[0]

        mouse_pos = pygame.mouse.get_pos()
        mouse_press = pygame.mouse.get_pressed(3)

        if mouse_press[0]:
            pygame.draw.rect(canvas, color, (mouse_pos[0] // pixel_size,
                                             mouse_pos[1] // pixel_size,
                                             1, 1))
        elif mouse_press[2]:
            pygame.draw.rect(canvas, secondary, (mouse_pos[0] // pixel_size,
                                                 mouse_pos[1] // pixel_size,
                                                 1, 1))
        elif mouse_press[1]:
            pos = (mouse_pos[0] // pixel_size, mouse_pos[1] // pixel_size)
            c = canvas.get_at(pos)
            c = (c[0], c[1], c[2], 0)
            canvas.set_at(pos, c)

        big = pygame.transform.scale(canvas, (s, s))
        screen.blit(big, (0, 0))

        if mouse_pos[0] < s and mouse_pos[1] < s:
            pygame.draw.rect(screen, (255, 255, 255),
                             (mouse_pos[0] // pixel_size * pixel_size,
                              mouse_pos[1] // pixel_size * pixel_size,
                              pixel_size, pixel_size))

        if size[0] > size[1]:
            x = s + MARGIN
            y = 0 + MARGIN
        else:
            x = 0 + MARGIN
            y = s + MARGIN
        preview = pygame.transform.scale(canvas, (CANVAS_SIZE[0] * preview_scale, CANVAS_SIZE[1] * preview_scale))

        if tile_mode == '':
            screen.blit(preview, (x, y))

            y += CANVAS_SIZE[1] * preview_scale
        elif tile_mode == 'tile':
            for i in range(3):
                for j in range(3):
                    screen.blit(preview, (x + i * CANVAS_SIZE[0] * preview_scale,
                                          y + j * CANVAS_SIZE[1] * preview_scale))

            y += 3 * CANVAS_SIZE[1] * preview_scale

        elif tile_mode == 'adapt':
            adapt = [
                (0, 0 + 2, 0 + 2),
                (1, 0 + 2, 1 + 2),
                (2, 0 + 2, 2 + 2),
                (3, 1 + 2, 0 + 2),
                (4, 1 + 2, 1 + 2),
                (5, 1 + 2, 2 + 2),
                (6, 2 + 2, 0 + 2),
                (7, 2 + 2, 1 + 2),
                (8, 2 + 2, 2 + 2),

                (9, 0, 0),

                (10, 2, 0),
                (11, 3, 0),
                (12, 4, 0),

                (13, 0, 2),
                (14, 0, 3),
                (15, 0, 4),

                (4, 6, 0),
                (4, 8, 0),
                (4, 6, 2),
                (4, 8, 2),

                (16, 6, 0),
                (17, 8, 0),
                (18, 6, 2),
                (19, 8, 2),

                (1, 2 + 6, 1),
                (3, 1 + 6, 2),
                (5, 1 + 6, 0),
                (7, 0 + 6, 1),
            ]
            for idx, i, j in adapt:
                dest = (x + i * CANVAS_SIZE[0] * preview_scale, y + j * CANVAS_SIZE[1] * preview_scale)

                preview = pygame.transform.scale(adapt_tiles[idx],
                                                 (CANVAS_SIZE[0] * preview_scale, CANVAS_SIZE[1] * preview_scale))

                screen.blit(preview, dest)

                if (
                    dest[0] < mouse_pos[0] < dest[0] + CANVAS_SIZE[0] * preview_scale
                    and dest[1] < mouse_pos[1] < dest[1] + CANVAS_SIZE[1] * preview_scale
                ):
                    # pygame.draw.rect(screen, (255, 255, 255),
                    #                  ((mouse_pos[0] - dest[0])
                    #                   // (CANVAS_SIZE[0] * preview_scale) * (CANVAS_SIZE[1] * preview_scale) + dest[0],
                    #                   (mouse_pos[1] - dest[1])
                    #                   // (CANVAS_SIZE[1] * preview_scale) * (CANVAS_SIZE[1] * preview_scale) + dest[1],
                    #                   CANVAS_SIZE[0] * preview_scale, CANVAS_SIZE[1] * preview_scale))
                    pygame.draw.rect(screen, (255, 255, 255), (dest[0], dest[1],
                                                               CANVAS_SIZE[0] * preview_scale,
                                                               CANVAS_SIZE[0] * preview_scale))

                    if mouse_press[0]:
                        canvas = adapt_tiles[idx]

            # preview = pygame.transform.scale(adapt_tiles[9],
            #                                  (CANVAS_SIZE[0] * preview_scale, CANVAS_SIZE[1] * preview_scale))
            # dest = (x + 4 * CANVAS_SIZE[0] * preview_scale, y + 1 * CANVAS_SIZE[1] * preview_scale)
            # screen.blit(preview, dest)
            #
            # if (
            #         dest[0] < mouse_pos[0] < dest[0] + CANVAS_SIZE[0] * preview_scale
            #         and dest[1] < mouse_pos[1] < dest[1] + CANVAS_SIZE[1] * preview_scale
            # ):
            #     pygame.draw.rect(screen, (255, 255, 255),
            #                      ((mouse_pos[0] - dest[0])
            #                       // (CANVAS_SIZE[0] * preview_scale) * (CANVAS_SIZE[1] * preview_scale) + dest[0],
            #                       (mouse_pos[1] - dest[1])
            #                       // (CANVAS_SIZE[1] * preview_scale) * (CANVAS_SIZE[1] * preview_scale) + dest[1],
            #                       CANVAS_SIZE[0] * preview_scale, CANVAS_SIZE[1] * preview_scale))
            #
            #     if mouse_press[0]:
            #         canvas = adapt_tiles[9]

            y += 5 * CANVAS_SIZE[1] * preview_scale

        else:
            assert False, f"unimplemented tile mode '{tile_mode}'"

        if zoom_timer < 60:
            screen.blit(font.render(str(preview_scale), True, (255, 255, 255)), (x, y + MARGIN))

        # TODO: palette picker in vertical window
        x = s + MARGIN
        y = s - pixel_size - MARGIN
        for c in palette:
            new_c = c
            if (
                    x <= mouse_pos[0] < x + pixel_size
                    and y <= mouse_pos[1] < y + pixel_size
            ):
                if mouse_press[0]:
                    color = c
                if mouse_press[2]:
                    secondary = c

                highlight = 70
                new_c = (c[0] + highlight, c[1] + highlight, c[2] + highlight)
                new_c = (min(new_c[0], 255), min(new_c[1], 255), min(new_c[2], 255))

            pygame.draw.rect(screen, new_c, (x, y, pixel_size, pixel_size))
            if color == secondary == c:
                pygame.draw.rect(screen, (0, 0, 0), (x, y, pixel_size // 2, MARGIN // 2))
                pygame.draw.rect(screen, (0, 0, 0), (x, y + pixel_size - MARGIN // 2, pixel_size // 2, MARGIN // 2))
                pygame.draw.rect(screen, (0, 0, 0), (x, y, MARGIN // 2, pixel_size))

                pygame.draw.rect(screen, (255, 255, 255), (x + pixel_size // 2, y, pixel_size // 2, MARGIN // 2))
                pygame.draw.rect(screen, (255, 255, 255),
                                 (x + pixel_size // 2, y + pixel_size - MARGIN // 2, pixel_size // 2, MARGIN // 2))
                pygame.draw.rect(screen, (255, 255, 255), (x + pixel_size - MARGIN // 2, y, MARGIN // 2, pixel_size))
            else:
                if color == c:
                    pygame.draw.rect(screen, (0, 0, 0), (x, y, pixel_size, pixel_size), MARGIN // 2, 1)
                if secondary == c:
                    pygame.draw.rect(screen, (255, 255, 255), (x, y, pixel_size, pixel_size), MARGIN // 2, 1)

            x += pixel_size
            if x + pixel_size > size[0]:  # the right side is past the screen
                x = s + MARGIN
                y -= pixel_size

        # if mouse_pos[1] - (pixel_size if x == s + MARGIN else 0) > y:
        #     pygame.draw.rect(screen, (255, 255, 255),
        #                      ((mouse_pos[0] - MARGIN) // pixel_size * pixel_size + MARGIN,
        #                       (mouse_pos[1] + MARGIN) // pixel_size * pixel_size - MARGIN,
        #                       pixel_size, pixel_size))

    elif state == 'ctrl_p':  # TODO: make this a list
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False
            elif event.type == pygame.KEYUP:
                if event.key == pygame.K_ESCAPE:
                    state = state_stack.pop()

                if event.mod & pygame.KMOD_CTRL:
                    if event.key == pygame.K_l:
                        state = 'load_palette'
                        state_stack.append(state)
                    elif event.key == pygame.K_s:
                        state = 'save_palette'
                        state_stack.append(state)
        screen.fill((50, 50, 50))

        screen.blit(font.render('ctrl+l load palette', True, (255, 255, 255)), (0, 0))
        screen.blit(font.render('ctrl+s save palette', True, (255, 255, 255)), (0, font_size))

    elif state == 'load_palette':
        if len(data_stack) == 0:
            state = 'choose_file'
        else:
            try:
                s = pygame.image.load(data_stack.pop())
            except pygame.error as e:
                state = 'error'
                data_stack.append(f"error loading image: {e}")
                continue

            palette = []

            for i in range(s.get_width()):
                palette.append(s.get_at((i, 0)))

            state = state_stack.pop()

            # raise NotImplementedError('load palette')

    elif state == 'save_palette':
        raise NotImplementedError('saving palette, TODO: implement if palette can be edited')

    elif state == 'choose_file':  # can not be called recursively
        if dir_i < 0:
            dir_i = 0
        elif dir_i >= len(dir_list):
            dir_i = len(dir_list) - 1

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False
            elif event.type == pygame.KEYUP:
                if event.key == pygame.K_PERIOD:
                    working_dir += '/..'
                    dir_list = sorted(os.listdir(working_dir))
                elif event.key == pygame.K_ESCAPE:
                    data_stack.append(None)
                    state = state_stack.pop()

                    dir_i = 0
                    working_dir = '.'
                    dir_list = sorted(os.listdir(working_dir))

                elif event.key == pygame.K_DOWN:
                    dir_i += 1
                elif event.key == pygame.K_UP:
                    dir_i -= 1

                elif event.key == pygame.K_RETURN:
                    if not dir_list:
                        continue

                    p = working_dir + '/' + dir_list[dir_i]
                    # print(p)
                    if os.path.isdir(p):
                        working_dir = p
                        dir_list = sorted(os.listdir(working_dir))
                    else:
                        data_stack.append(p)
                        state = state_stack.pop()

                        dir_i = 0
                        working_dir = '.'
                        dir_list = sorted(os.listdir(working_dir))

                elif event.mod & pygame.KMOD_CTRL and event.key == pygame.K_n:
                    dir_i = 0
                    if event.mod & pygame.KMOD_SHIFT:
                        state_stack.append(state)
                        state = 'new_dir'
                    else:
                        state = 'new_file'

        screen.fill((50, 50, 50))

        y = 0
        for i, f in enumerate(dir_list):
            if i == dir_i:
                pygame.draw.rect(screen, (100, 100, 100), (0, y, size[0], font_size))
            screen.blit(font.render(f, True, (255, 255, 255)), (0, y))
            y += font_size

    elif state == 'error':
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False
            elif event.type == pygame.KEYUP:
                pass
        screen.fill((50, 50, 50))

        # TODO: error messages can be very long and go of the screen
        screen.blit(font.render(data_stack[-1], True, (255, 255, 255)), (0, 0))

    elif state == 'ctrl_t':
        if dir_i < 0:
            dir_i = 0
        elif dir_i >= len(tile_modes):
            dir_i = len(tile_modes) - 1

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False
            elif event.type == pygame.KEYUP:
                if event.key == pygame.K_ESCAPE:
                    state = state_stack.pop()

                elif event.key == pygame.K_DOWN:
                    dir_i += 1
                elif event.key == pygame.K_UP:
                    dir_i -= 1

                elif event.key == pygame.K_RETURN:
                    tile_mode = tile_modes[dir_i]
                    dir_i = 0

                    state = state_stack.pop()

        screen.fill((50, 50, 50))

        y = 0
        for i, mode in enumerate(tile_modes):
            if i == dir_i:
                pygame.draw.rect(screen, (100, 100, 100), (0, y, size[0], font_size))

            screen.blit(font.render(mode, True, (255, 255, 255)), (0, y))
            y += font_size

    elif state == 'ctrl_s':
        if len(data_stack) == 0:
            state_stack.append(state)
            state = 'choose_file'
        else:
            name = data_stack.pop()
            if tile_mode == 'adapt':
                for i, tile in enumerate(adapt_tiles):
                    pygame.image.save(tile, f'{name} {i}.png')
            else:
                pygame.image.save(canvas, f'{name}.png')

            state = state_stack.pop()

    elif state == 'new_file':
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False
            elif event.type == pygame.TEXTINPUT:
                choose_file_file_name += event.text

            elif event.type == pygame.KEYUP:
                if event.key == pygame.K_RETURN:
                    data_stack.append(working_dir + '/' + choose_file_file_name)
                    choose_file_file_name = ''

                    state = state_stack.pop()

        screen.fill((50, 50, 50))

        screen.blit(font.render('file name:', True, (255, 255, 255)), (0, 0))
        screen.blit(font.render(choose_file_file_name, True, (255, 255, 255)), (0, font_size))

    elif state == 'new_dir':
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False
            elif event.type == pygame.TEXTINPUT:
                choose_file_file_name += event.text

            elif event.type == pygame.KEYUP:
                if event.key == pygame.K_RETURN:
                    os.mkdir(working_dir + '/' + choose_file_file_name)
                    dir_list = sorted(os.listdir(working_dir))

                    choose_file_file_name = ''

                    state = state_stack.pop()

        screen.fill((50, 50, 50))

        screen.blit(font.render('dir name:', True, (255, 255, 255)), (0, 0))
        screen.blit(font.render(choose_file_file_name, True, (255, 255, 255)), (0, font_size))

    else:
        assert False, f"unknown state '{state}' in stack {state_stack}"

    # print(state, state_stack)

    zoom_timer += 1
    pygame.display.update()

pygame.quit()
