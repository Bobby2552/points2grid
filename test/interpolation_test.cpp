#include <gtest/gtest.h>
#include <points2grid/Interpolation.hpp>

#include <points2grid/config.h>
#include <points2grid/Global.hpp>

#include "fixtures.hpp"


namespace points2grid
{


namespace
{


struct AscHeader
{
    int ncols;
    int nrows;
    double xllcorner;
    double yllcorner;
    double cellsize;
    int NODATA_value;
};

struct GridHeader
{
    double north;
    double south;
    double east;
    double west;
    int rows;
    int cols;
};

struct AsciiData
{
    AsciiData(int numvals) : values(0) {
        if (numvals > 0) values = new double[numvals];
    }
    ~AsciiData() {
        if (values) delete[] values;
    }

    double *values;
};


AscHeader read_asc_header(std::istream& is)
{
    AscHeader header;
    std::streamsize n = std::numeric_limits<std::streamsize>::max();
    is.ignore(n, ' ');
    is >> header.ncols;
    is.ignore(n, ' ');
    is >> header.nrows;
    is.ignore(n, ' ');
    is >> header.xllcorner;
    is.ignore(n, ' ');
    is >> header.yllcorner;
    is.ignore(n, ' ');
    is >> header.cellsize;
    is.ignore(n, ' ');
    is >> header.NODATA_value;
    return header;
}

GridHeader read_grid_header(std::istream& is)
{
    GridHeader header;
    std::streamsize n = std::numeric_limits<std::streamsize>::max();
    is.ignore(n, ' ');
    is >> header.north;
    is.ignore(n, ' ');
    is >> header.south;
    is.ignore(n, ' ');
    is >> header.east;
    is.ignore(n, ' ');
    is >> header.west;
    is.ignore(n, ' ');
    is >> header.rows;
    is.ignore(n, ' ');
    is >> header.cols;
    return header;
}

AsciiData read_asc_data(std::istream &is)
{
    AscHeader hdr = read_asc_header(is);
    AsciiData data (hdr.ncols*hdr.nrows);

    for (int i=0; i<hdr.ncols*hdr.nrows; ++i)
        is >> data.values[i];

    return data;
}

AsciiData read_grid_data(std::istream &is)
{
    GridHeader hdr = read_grid_header(is);
    AsciiData data (hdr.cols*hdr.rows);

    for (int i=0; i<hdr.cols*hdr.rows; ++i)
        is >> data.values[i];

    return data;
}


class InterpolationTest : public FourPointsTest
{};


}


TEST_F(InterpolationTest, Constructor)
{
    Interpolation interp(1, 1, 1, 3, INTERP_INCORE);
}


TEST_F(InterpolationTest, InitAscii)
{
    Interpolation interp(1, 1, 1, 3, INTERP_INCORE);
    int retval = interp.init(infile, INPUT_ASCII);
    EXPECT_EQ(0, retval);
}


TEST_F(InterpolationTest, Interpolate)
{
    Interpolation interp(1, 1, 1, 3, INTERP_INCORE);
    interp.init(infile, INPUT_ASCII);
    int retval = interp.interpolation(infile, outfile, INPUT_ASCII, OUTPUT_FORMAT_ARC_ASCII, OUTPUT_TYPE_ALL);
    EXPECT_EQ(0, retval);
    EXPECT_EQ(4U, interp.getDataCount());
    EXPECT_EQ(2U, interp.getGridSizeX());
    EXPECT_EQ(2U, interp.getGridSizeY());
}


TEST_F(InterpolationTest, ArcGISHeaders)
{
    Interpolation interp(1, 1, 1, 3, INTERP_INCORE);
    interp.init(infile, INPUT_ASCII);
    interp.interpolation(infile, outfile, INPUT_ASCII, OUTPUT_FORMAT_ALL, OUTPUT_TYPE_ALL);

    // Test ArcGIS headers
    std::ifstream asc;
    asc.open((outfile + ".idw.asc").c_str());
    AscHeader asc_header = read_asc_header(asc);
    EXPECT_EQ(asc_header.ncols, 2);
    EXPECT_EQ(asc_header.nrows, 2);
    EXPECT_DOUBLE_EQ(asc_header.xllcorner, 0.5);
    EXPECT_DOUBLE_EQ(asc_header.yllcorner, 0.5);
    EXPECT_DOUBLE_EQ(asc_header.cellsize, 1.0);
    EXPECT_EQ(asc_header.NODATA_value, -9999);
}


TEST_F(InterpolationTest, GridHeaders)
{
    Interpolation interp(1, 1, 1, 3, INTERP_INCORE);
    interp.init(infile, INPUT_ASCII);
    interp.interpolation(infile, outfile, INPUT_ASCII, OUTPUT_FORMAT_ALL, OUTPUT_TYPE_ALL);

    // Test GRID headers
    std::ifstream grid;
    grid.open((outfile + ".idw.grid").c_str());
    GridHeader grid_header = read_grid_header(grid);
    EXPECT_DOUBLE_EQ(grid_header.north, 2.5);
    EXPECT_DOUBLE_EQ(grid_header.south, 0.5);
    EXPECT_DOUBLE_EQ(grid_header.east, 2.5);
    EXPECT_DOUBLE_EQ(grid_header.west, 0.5);
    EXPECT_EQ(grid_header.rows, 2);
    EXPECT_EQ(grid_header.cols, 2);
}


TEST_F(InterpolationTest, ArcGISHeadersUserGrid)
{
    Interpolation interp(1, 1, 0.01, 3, INTERP_INCORE);
    interp.init(infile, 3.5, -0.5, 4.5, -1.5);
    interp.interpolation(infile, outfile, INPUT_ASCII, OUTPUT_FORMAT_ALL, OUTPUT_TYPE_ALL);

    std::ifstream asc;
    asc.open((outfile + ".mean.asc").c_str());
    AscHeader asc_header = read_asc_header(asc);
    EXPECT_EQ(asc_header.ncols, 6);
    EXPECT_EQ(asc_header.nrows, 4);
    EXPECT_DOUBLE_EQ(asc_header.xllcorner, -1.5);
    EXPECT_DOUBLE_EQ(asc_header.yllcorner, -0.5);
    EXPECT_DOUBLE_EQ(asc_header.cellsize, 1.0);
    EXPECT_EQ(asc_header.NODATA_value, -9999);
}


TEST_F(InterpolationTest, GridHeadersUserGrid)
{
    Interpolation interp(1, 1, 0.01, 3, INTERP_INCORE);
    interp.init(infile, 3.5, -0.5, 4.5, -1.5);
    interp.interpolation(infile, outfile, INPUT_ASCII, OUTPUT_FORMAT_ALL, OUTPUT_TYPE_ALL);

    std::ifstream grid;
    grid.open((outfile + ".mean.grid").c_str());
    GridHeader grid_header = read_grid_header(grid);
    EXPECT_DOUBLE_EQ(grid_header.north, 3.5);
    EXPECT_DOUBLE_EQ(grid_header.south, -0.5);
    EXPECT_DOUBLE_EQ(grid_header.east, 4.5);
    EXPECT_DOUBLE_EQ(grid_header.west, -1.5);
    EXPECT_EQ(grid_header.rows, 4);
    EXPECT_EQ(grid_header.cols, 6);
}


TEST_F(InterpolationTest, ValuesUserGridLarge)
{
    Interpolation interp(1, 1, 0.01, 0, INTERP_INCORE);
    interp.init(infile, 3.5, -0.5, 4.5, -1.5);
    interp.interpolation(infile, outfile, INPUT_ASCII, OUTPUT_FORMAT_ALL, OUTPUT_TYPE_ALL);

    double solution [] = {-9999, -9999, -9999, -9999, -9999, -9999,
                          -9999, -9999, 4.000, 1.000, -9999, -9999,
                          -9999, -9999, 3.000, 2.000, -9999, -9999,
                          -9999, -9999, -9999, -9999, -9999, -9999 };

    // Test ArcGIS data
    std::ifstream asc;
    asc.open((outfile + ".mean.asc").c_str());
    AsciiData ascdata = read_asc_data(asc);
    for (int i=0; i<24; ++i)
        EXPECT_DOUBLE_EQ(ascdata.values[i], solution[i]);

    // Test GRID data
    std::ifstream grid;
    grid.open((outfile + ".mean.grid").c_str());
    AsciiData griddata = read_grid_data(grid);
    for (int i=0; i<24; ++i)
        EXPECT_DOUBLE_EQ(griddata.values[i], solution[i]);
}


TEST_F(InterpolationTest, ValuesUserGridSmall)
{
    Interpolation interp(1, 1, 0.01, 0, INTERP_INCORE);
    interp.init(infile, 1.5, 0.5, 1.5, 0.5);
    interp.interpolation(infile, outfile, INPUT_ASCII, OUTPUT_FORMAT_ALL, OUTPUT_TYPE_ALL);

    // Test ArcGIS data
    std::ifstream asc;
    asc.open((outfile + ".mean.asc").c_str());
    AsciiData ascdata = read_asc_data(asc);
    EXPECT_DOUBLE_EQ(ascdata.values[0], 3.0);

    // Test GRID data
    std::ifstream grid;
    grid.open((outfile + ".mean.grid").c_str());
    AsciiData griddata = read_grid_data(grid);
    EXPECT_DOUBLE_EQ(griddata.values[0], 3.0);
}


}
