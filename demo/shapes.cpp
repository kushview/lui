// Copyright 2019 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include <lui/path.hpp>

#include "demo.hpp"

namespace lui {
namespace demo {

class Shapes : public DemoWidget {
public:
    Shapes() {
        set_size (640, 360);
    }

    ~Shapes() = default;

protected:
    void paint (Graphics& g) override {
        DemoWidget::paint (g);

        auto r          = bounds().at (0);
        auto center_y   = r.height / 2.0f;
        auto shape_size = 100.0f;
        auto spacing    = 150.0f;
        auto start_x    = 100.0f;

        // White filled circle
        g.set_color (Color (0xffffffff));
        Path circle_fill;
        circle_fill.add_ellipse (start_x, center_y - shape_size / 2, shape_size, shape_size);
        g.fill_path (circle_fill);

        // Red circle stroke
        g.set_color (Color (0xffff0000));
        g.context().set_line_width (3.0);
        Path circle_stroke;
        circle_stroke.add_ellipse (start_x, center_y - shape_size / 2, shape_size, shape_size);
        g.stroke_path (circle_stroke);

        // Green rectangle
        g.set_color (Color (0xff00ff00));
        g.fill_rect ((int) (start_x + spacing), (int) (center_y - shape_size / 2), (int) shape_size, (int) shape_size);

        // Cyan triangle
        g.set_color (Color (0xff00ffff));
        Path triangle;
        float tri_x = start_x + spacing * 2 + shape_size / 2;
        float tri_y = center_y;
        float tri_h = shape_size * 0.866f; // equilateral triangle height
        triangle.move_to (tri_x, tri_y - tri_h / 2);
        triangle.line_to (tri_x + shape_size / 2, tri_y + tri_h / 2);
        triangle.line_to (tri_x - shape_size / 2, tri_y + tri_h / 2);
        triangle.close_path();
        g.fill_path (triangle);
    }
};

std::unique_ptr<Widget> create_shapes_demo() {
    return std::make_unique<Shapes>();
}

} // namespace demo
} // namespace lui
