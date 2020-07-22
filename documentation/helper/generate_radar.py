import enum
import itertools
import statistics
from pathlib import Path

from plotly import graph_objects


class Cube(enum.IntEnum):
    EMPTY = 0
    SOLID = 1
    NORMAL = 2


def oc_get_values(func):
    cube_types = [Cube.EMPTY, Cube.SOLID, Cube.NORMAL]
    values = [0, 0, 0]
    for idx in range(len(cube_types)):
        values[idx] = func(cube_types[idx])
    return values


def oc_min(form):
    values = oc_get_values(form.min)

    val = min(values)
    return 8 * val


def oc_avg(form):
    def iter_f():
        itr = itertools.combinations_with_replacement(range(3), 8)
        next(itr)  # drop empty cube
        for cubes in itertools.combinations_with_replacement(range(3), 8):
            if cubes == (2, 2, 2, 2, 2, 2, 2, 2):
                continue  # drop SOLID empty
            c_sum = 0
            for cube in cubes:
                c_sum += form.avg(cube)
            yield c_sum / 8

    return statistics.median(iter_f())


def oc_max(form):
    values = oc_get_values(form.max)

    val = max(values)
    return 8 * val


class Sauerbraten:
    @staticmethod
    def name():
        return "Sauerbraten"

    @classmethod
    def min(cls, c_type):
        if c_type == Cube.EMPTY:
            return 8
        elif c_type == Cube.SOLID:
            return 8
        elif c_type == Cube.NORMAL:
            return 8 + 12 * 8

    @classmethod
    def avg(cls, c_type):
        if c_type == Cube.EMPTY:
            return 8
        elif c_type == Cube.SOLID:
            return 8
        elif c_type == Cube.NORMAL:
            return 8 + 12 * 8

    @classmethod
    def max(cls, c_type):
        if c_type == Cube.EMPTY:
            return 8
        elif c_type == Cube.SOLID:
            return 8
        elif c_type == Cube.NORMAL:
            return 8 + 12 * 8


class InexorI:
    @staticmethod
    def name():
        return "Inexor I"

    @staticmethod
    def calc(value):
        """NORMAL cube size"""
        return 2 + 8 * 3 + value * 3

    @classmethod
    def min(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            return cls.calc(1)

    @classmethod
    def avg(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            def iter_f():
                for i in range(1, 3 * 8 + 1):
                    yield cls.calc(i)

            return statistics.median(iter_f())

    @classmethod
    def max(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            return cls.calc(8 * 3)


class InexorII:
    @staticmethod
    def name():
        return "Inexor II"

    @staticmethod
    def calc(one_sided, double_sided):
        """NORMAL cube size"""
        return 2 + 12 * 2 + one_sided * 3 + double_sided * 5

    @classmethod
    def min(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            return cls.calc(1, 0)

    @classmethod
    def avg(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            def iter_f():
                itr = itertools.combinations_with_replacement(range(3), 12)
                next(itr)
                for i in itr:
                    values = i
                    yield cls.calc(values.count(1), values.count(2))

            return statistics.median(iter_f())

    @classmethod
    def max(cls, c_type):
        if c_type == Cube.EMPTY:
            return 2
        elif c_type == Cube.SOLID:
            return 2
        elif c_type == Cube.NORMAL:
            return cls.calc(0, 12)


class InexorIII:
    @staticmethod
    def name():
        return "Inexor III"

    @classmethod
    def min(cls, c_type):
        if c_type == Cube.EMPTY:
            return 8
        elif c_type == Cube.SOLID:
            return 8
        elif c_type == Cube.NORMAL:
            return 8 + 12 * 6

    @classmethod
    def avg(cls, c_type):
        if c_type == Cube.EMPTY:
            return 8
        elif c_type == Cube.SOLID:
            return 8
        elif c_type == Cube.NORMAL:
            return 8 + 12 * 6

    @classmethod
    def max(cls, c_type):
        if c_type == Cube.EMPTY:
            return 8
        elif c_type == Cube.SOLID:
            return 8
        elif c_type == Cube.NORMAL:
            return 8 + 12 * 6


def generate_raw_data(formats):
    raw_data = {}
    for format in formats:
        values = []
        for typ in [Cube.EMPTY, Cube.SOLID, Cube.NORMAL]:
            values.extend([format.min(typ), format.avg(typ), format.max(typ)])
        values.extend([oc_min(format), oc_avg(format), oc_max(format)])
        raw_data[format] = values
    return raw_data


def get_plot_values(raw_data, relative_to=None):
    """
    :param relative_to: if None, absolute values are used
    :return: radial coordinate list
    """
    plot_values = []
    minimum_r = list(raw_data.values())[0][0]
    maximum_r = minimum_r

    for format, values in raw_data.items():
        if relative_to is not None:
            radial_coords = list(map(lambda a, b: 100 / a * b, relative_to, values))
        else:
            radial_coords = values

        minimum_r = min(min(radial_coords), minimum_r)
        maximum_r = max(max(radial_coords), maximum_r)

        text_list = values
        # add unit to text, which shows the absolute values
        text_list = list(map(lambda val: str(val) + ' bits', text_list))

        # connect last to first value, to get the line between them
        radial_coords.append(radial_coords[0])
        text_list.append(values[0])

        plot_values.append((format.name(), radial_coords, text_list))

    return plot_values, minimum_r, maximum_r


def plotly(formats, categories, relative_to_format=None):
    fig = graph_objects.Figure()

    # list of formats and their values
    raw_data = generate_raw_data(formats)

    # first diagram
    relative_to = None
    if relative_to_format is not None:
        relative_to = raw_data[relative_to_format]

    # connect last to first value, to get the line between them
    categories.append(categories[0])
    plot_values, minimum_r, maximum_r = get_plot_values(raw_data, relative_to)

    for name, radial_coords, text_list in plot_values:
        fig.add_trace(graph_objects.Scatterpolar(
            name=name,
            r=radial_coords,
            text=text_list,
            theta=categories,
        ))

    if relative_to_format is not None:
        plot_title = f"Relative structure size in comparison to {relative_to_format.name()}"
    else:
        plot_title = "Absolute structure size in bits"

    fig.update_layout(
        title=plot_title,
        polar=dict(
            radialaxis=dict(
                visible=True,
                range=[minimum_r / 2, maximum_r],
            )),
        showlegend=True
    )
    return fig


def generate(output_folder=Path("./")):
    output_folder.mkdir(parents=True, exist_ok=True)

    formats = [Sauerbraten, InexorI, InexorII, InexorIII]
    categories = ['Empty (min)', 'Empty (avg)', 'Empty (max)',
                  'Solid (min)', 'Solid (avg)', 'Solid (max)',
                  'Normal (min)', 'Normal (avg)', 'Normal (max)',
                  'Octant (min)', 'Octant (avg)', 'Octant (max)']

    fig = plotly(formats, categories, relative_to_format=None)
    fig.write_html(str(output_folder.joinpath("radar.html")))
    fig = plotly(formats, categories, relative_to_format=InexorIII)
    fig.write_html(str(output_folder.joinpath("radar_rel_inexor_iii.html")))
