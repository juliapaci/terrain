use macroquad::prelude::*;
extern crate rand;
use rand::Rng;

#[path = "perlin/perlin.rs"]
mod perlin;

const SIZE: usize = 10000;

#[macroquad::main("Terrain")]
async fn main() {
    let perlin = get_perlin();

    let mut perlin_bytes: Vec<u8> = Vec::with_capacity(SIZE*4);
    for val in perlin.iter() {
        let colour: u8 = (((val + 1.)*0.5) * 255.) as u8;


        perlin_bytes.push(colour);  // R
        perlin_bytes.push(colour);  // G
        perlin_bytes.push(colour);  // B
        perlin_bytes.push(255);     // A
    }
    println!("{:?}", perlin_bytes);

    let perlin_texture = Texture2D::from_rgba8((SIZE as f64).sqrt() as u16, (SIZE as f64).sqrt() as u16, &perlin_bytes[..]);


    loop {
        clear_background(GRAY);
        // draw_texture(&perlin_texture, 0., 0., WHITE);
        let params = DrawTextureParams {
            dest_size: Some(Vec2 {
                x: 1000.,
                y: 1000.
            }),
            source: None,
            rotation: 0.,
            flip_x: false,
            flip_y: false,
            pivot: None
        };
        draw_texture_ex(&perlin_texture, 0., 0., WHITE, params);

        next_frame().await
    }
}

fn get_perlin() -> [f32; SIZE] {
    let mut perlin: [f32; SIZE] = [0.; SIZE];

    for i in 0..SIZE  {
        perlin[i] = perlin::perlin((
                rand::thread_rng().gen_range(0.0..100.),
                rand::thread_rng().gen_range(0.0..100.)
            ));
    }

    perlin
}
