#[macro_use]
extern crate actix_web;
use std::{env, io};
use actix_web::{ App, HttpResponse, HttpServer };

#[get("/")]
async fn hello() -> HttpResponse {
    HttpResponse::Ok()
        .content_type("text/plain")
        .body("Hello World!")
}

#[actix_rt::main]
async fn main() -> io::Result<()> {
    env::set_var("RUST_LOG", "actix_web=debug,actix_server=info");
    env_logger::init();

    HttpServer::new(|| {
        App::new()
            .service(hello)
    })
    .bind("127.0.0.1:8000")?
    .run()
    .await
}
