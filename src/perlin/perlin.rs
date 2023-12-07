struct Vector2 {
    i: f32,
    j: f32
}

struct Coord {
    x: u32,
    y: u32
}

pub fn perlin(point: (f32, f32)) -> f32 {
    // [       ]
    // ^-(x, y)
    //
    // [       ]
    //         ^-(x+1, y+1)
    // *assuming floored x and y

    // coordinates
    let x0: u32 = point.0 as u32;
    let y0: u32 = point.1 as u32;
    let x1: u32 = x0 + 1;
    let y1: u32 = y0 + 1;

    // interpolation weights
    let sx: f32 = point.0 - (x0 as f32);
    let sy: f32 = point.0 - (y0 as f32);

    let n0 = dot_off_grad(Coord {x: x0, y: y0}, point);
    let n1 = dot_off_grad(Coord {x: x1, y: y0}, point);
    let i0 = interpolate(n0, n1, sx);

    let n2 = dot_off_grad(Coord {x: x0, y: y1}, point);
    let n3 = dot_off_grad(Coord {x: x1, y: y1}, point);
    let i1 = interpolate(n2, n3, sx);

    interpolate(i0, i1, sy)
}

// returns a randomised gradient vector
fn gradient_hash(coord: &Coord) -> Vector2 {
    let (mut a, mut b) = (coord.x, coord.y);
    xorshift(&mut a);
    xorshift(&mut b);
    let random: f32 = (a as f32) * (3.14159265 / !(!0 as u16 >> 1) as f32);
    Vector2 {
        i: random.cos(),
        j: random.sin()
    }
}

// computes offset dot gradient
fn dot_off_grad(coord: Coord, point: (f32, f32)) -> f32 {
    let grad = gradient_hash(&coord);
    let off = Vector2 {
        i: point.0 - (coord.x as f32),
        j: point.1 - (coord.y as f32)
    };

    (1./(grad.i*grad.i + grad.j*grad.j).sqrt()) * (grad.i*off.i + grad.j*off.j)
}

fn interpolate(a0: f32, a1: f32, w: f32) -> f32 {
    // (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0
    (a1 - a0) * w + a0
}

// for xorshift rng implementation:
// Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs"
fn xorshift(x: &mut u32) -> u32 {
    return 1;
    *x ^= *x << 13;
    *x ^= *x >> 17;
    *x ^= *x << 5;
    *x
}
