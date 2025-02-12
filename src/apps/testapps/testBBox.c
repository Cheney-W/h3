/*
 * Copyright 2016-2018, 2020-2021 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <math.h>
#include <stdlib.h>

#include "bbox.h"
#include "constants.h"
#include "latLng.h"
#include "polygon.h"
#include "test.h"

void assertBBox(const GeoLoop *geoloop, const BBox *expected,
                const LatLng *inside, const LatLng *outside) {
    BBox result;

    bboxFromGeoLoop(geoloop, &result);

    t_assert(bboxEquals(&result, expected), "Got expected bbox");
    t_assert(bboxContains(&result, inside), "Contains expected inside point");
    t_assert(!bboxContains(&result, outside),
             "Does not contain expected outside point");
}

SUITE(BBox) {
    TEST(posLatPosLng) {
        LatLng verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {1.1, 0.7, 0.7, 0.2};
        const LatLng inside = {0.9, 0.4};
        const LatLng outside = {0.0, 0.0};
        assertBBox(&geoloop, &expected, &inside, &outside);
    }

    TEST(negLatPosLng) {
        LatLng verts[] = {{-0.3, 0.6}, {-0.4, 0.9}, {-0.2, 0.8}, {-0.1, 0.6}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {-0.1, -0.4, 0.9, 0.6};
        const LatLng inside = {-0.3, 0.8};
        const LatLng outside = {0.0, 0.0};
        assertBBox(&geoloop, &expected, &inside, &outside);
    }

    TEST(posLatNegLng) {
        LatLng verts[] = {{0.7, -1.4}, {0.8, -0.9}, {1.0, -0.8}, {1.1, -1.3}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {1.1, 0.7, -0.8, -1.4};
        const LatLng inside = {0.9, -1.0};
        const LatLng outside = {0.0, 0.0};
        assertBBox(&geoloop, &expected, &inside, &outside);
    }

    TEST(negLatNegLng) {
        LatLng verts[] = {
            {-0.4, -1.4}, {-0.3, -1.1}, {-0.1, -1.2}, {-0.2, -1.4}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {-0.1, -0.4, -1.1, -1.4};
        const LatLng inside = {-0.3, -1.2};
        const LatLng outside = {0.0, 0.0};
        assertBBox(&geoloop, &expected, &inside, &outside);
    }

    TEST(aroundZeroZero) {
        LatLng verts[] = {{0.4, -0.4}, {0.4, 0.4}, {-0.4, 0.4}, {-0.4, -0.4}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {0.4, -0.4, 0.4, -0.4};
        const LatLng inside = {-0.1, -0.1};
        const LatLng outside = {1.0, -1.0};
        assertBBox(&geoloop, &expected, &inside, &outside);
    }

    TEST(transmeridian) {
        LatLng verts[] = {{0.4, M_PI - 0.1},
                          {0.4, -M_PI + 0.1},
                          {-0.4, -M_PI + 0.1},
                          {-0.4, M_PI - 0.1}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {0.4, -0.4, -M_PI + 0.1, M_PI - 0.1};
        const LatLng insideOnMeridian = {-0.1, M_PI};
        const LatLng outside = {1.0, M_PI - 0.5};
        assertBBox(&geoloop, &expected, &insideOnMeridian, &outside);

        const LatLng westInside = {0.1, M_PI - 0.05};
        t_assert(bboxContains(&expected, &westInside),
                 "Contains expected west inside point");
        const LatLng eastInside = {0.1, -M_PI + 0.05};
        t_assert(bboxContains(&expected, &eastInside),
                 "Contains expected east outside point");

        const LatLng westOutside = {0.1, M_PI - 0.5};
        t_assert(!bboxContains(&expected, &westOutside),
                 "Does not contain expected west outside point");
        const LatLng eastOutside = {0.1, -M_PI + 0.5};
        t_assert(!bboxContains(&expected, &eastOutside),
                 "Does not contain expected east outside point");
    }

    TEST(edgeOnNorthPole) {
        LatLng verts[] = {{M_PI_2 - 0.1, 0.1},
                          {M_PI_2 - 0.1, 0.8},
                          {M_PI_2, 0.8},
                          {M_PI_2, 0.1}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {M_PI_2, M_PI_2 - 0.1, 0.8, 0.1};
        const LatLng inside = {M_PI_2 - 0.01, 0.4};
        const LatLng outside = {M_PI_2, 0.9};
        assertBBox(&geoloop, &expected, &inside, &outside);
    }

    TEST(edgeOnSouthPole) {
        LatLng verts[] = {{-M_PI_2 + 0.1, 0.1},
                          {-M_PI_2 + 0.1, 0.8},
                          {-M_PI_2, 0.8},
                          {-M_PI_2, 0.1}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {-M_PI_2 + 0.1, -M_PI_2, 0.8, 0.1};
        const LatLng inside = {-M_PI_2 + 0.01, 0.4};
        const LatLng outside = {-M_PI_2, 0.9};
        assertBBox(&geoloop, &expected, &inside, &outside);
    }

    TEST(containsEdges) {
        const BBox bbox = {0.1, -0.1, 0.2, -0.2};
        LatLng points[] = {
            {0.1, 0.2},  {0.1, 0.0},  {0.1, -0.2},  {0.0, 0.2},
            {-0.1, 0.2}, {-0.1, 0.0}, {-0.1, -0.2}, {0.0, -0.2},
        };
        const int numPoints = 8;

        for (int i = 0; i < numPoints; i++) {
            t_assert(bboxContains(&bbox, &points[i]), "Contains edge point");
        }
    }

    TEST(containsEdgesTransmeridian) {
        const BBox bbox = {0.1, -0.1, -M_PI + 0.2, M_PI - 0.2};
        LatLng points[] = {
            {0.1, -M_PI + 0.2}, {0.1, M_PI},         {0.1, M_PI - 0.2},
            {0.0, -M_PI + 0.2}, {-0.1, -M_PI + 0.2}, {-0.1, M_PI},
            {-0.1, M_PI - 0.2}, {0.0, M_PI - 0.2},
        };
        const int numPoints = 8;

        for (int i = 0; i < numPoints; i++) {
            t_assert(bboxContains(&bbox, &points[i]),
                     "Contains transmeridian edge point");
        }
    }

    TEST(bboxCenterBasicQuandrants) {
        LatLng center;

        BBox bbox1 = {1.0, 0.8, 1.0, 0.8};
        LatLng expected1 = {0.9, 0.9};
        bboxCenter(&bbox1, &center);
        t_assert(geoAlmostEqual(&center, &expected1), "pos/pos as expected");

        BBox bbox2 = {-0.8, -1.0, 1.0, 0.8};
        LatLng expected2 = {-0.9, 0.9};
        bboxCenter(&bbox2, &center);
        t_assert(geoAlmostEqual(&center, &expected2), "neg/pos as expected");

        BBox bbox3 = {1.0, 0.8, -0.8, -1.0};
        LatLng expected3 = {0.9, -0.9};
        bboxCenter(&bbox3, &center);
        t_assert(geoAlmostEqual(&center, &expected3), "pos/neg as expected");

        BBox bbox4 = {-0.8, -1.0, -0.8, -1.0};
        LatLng expected4 = {-0.9, -0.9};
        bboxCenter(&bbox4, &center);
        t_assert(geoAlmostEqual(&center, &expected4), "neg/neg as expected");

        BBox bbox5 = {0.8, -0.8, 1.0, -1.0};
        LatLng expected5 = {0.0, 0.0};
        bboxCenter(&bbox5, &center);
        t_assert(geoAlmostEqual(&center, &expected5),
                 "around origin as expected");
    }

    TEST(bboxCenterTransmeridian) {
        LatLng center;

        BBox bbox1 = {1.0, 0.8, -M_PI + 0.3, M_PI - 0.1};
        LatLng expected1 = {0.9, -M_PI + 0.1};
        bboxCenter(&bbox1, &center);

        t_assert(geoAlmostEqual(&center, &expected1), "skew east as expected");

        BBox bbox2 = {1.0, 0.8, -M_PI + 0.1, M_PI - 0.3};
        LatLng expected2 = {0.9, M_PI - 0.1};
        bboxCenter(&bbox2, &center);
        t_assert(geoAlmostEqual(&center, &expected2), "skew west as expected");

        BBox bbox3 = {1.0, 0.8, -M_PI + 0.1, M_PI - 0.1};
        LatLng expected3 = {0.9, M_PI};
        bboxCenter(&bbox3, &center);
        t_assert(geoAlmostEqual(&center, &expected3),
                 "on antimeridian as expected");
    }

    TEST(bboxIsTransmeridian) {
        BBox bboxNormal = {1.0, 0.8, 1.0, 0.8};
        t_assert(!bboxIsTransmeridian(&bboxNormal),
                 "Normal bbox not transmeridian");

        BBox bboxTransmeridian = {1.0, 0.8, -M_PI + 0.3, M_PI - 0.1};
        t_assert(bboxIsTransmeridian(&bboxTransmeridian),
                 "Transmeridian bbox is transmeridian");
    }

    TEST(bboxEquals) {
        BBox bbox = {1.0, 0.0, 1.0, 0.0};
        BBox north = bbox;
        north.north += 0.1;
        BBox south = bbox;
        south.south += 0.1;
        BBox east = bbox;
        east.east += 0.1;
        BBox west = bbox;
        west.west += 0.1;

        t_assert(bboxEquals(&bbox, &bbox), "Equals self");
        t_assert(!bboxEquals(&bbox, &north), "Not equals different north");
        t_assert(!bboxEquals(&bbox, &south), "Not equals different south");
        t_assert(!bboxEquals(&bbox, &east), "Not equals different east");
        t_assert(!bboxEquals(&bbox, &west), "Not equals different west");
    }

    TEST(bboxHexEstimate_invalidRes) {
        int64_t numHexagons;
        BBox bbox = {1.0, 0.0, 1.0, 0.0};
        t_assert(bboxHexEstimate(&bbox, -1, &numHexagons) == E_RES_DOMAIN,
                 "bboxHexEstimate of invalid resolution fails");
    }

    TEST(lineHexEstimate_invalidRes) {
        int64_t numHexagons;
        LatLng origin = {0.0, 0.0};
        LatLng destination = {1.0, 1.0};
        t_assert(lineHexEstimate(&origin, &destination, -1, &numHexagons) ==
                     E_RES_DOMAIN,
                 "lineHexEstimate of invalid resolution fails");
    }
}
