use std::convert::Infallible;
use hyper::service::{make_service_fn, service_fn};
use hyper::{Body, Request, Response, Server, Method, StatusCode, header};

async fn hello(req: Request<Body>) -> Result<Response<Body>, Infallible> {
	match (req.method(), req.uri().path()) {
		(&Method::GET, "/") => {
			Ok(Response::builder()
				.status(StatusCode::OK)
				.header(header::CONTENT_TYPE, "text/plain")
				.body(Body::from("Hello World!"))
				.unwrap())
		},
		_ => {
			Ok(Response::builder()
				.status(StatusCode::NOT_FOUND)
				.header(header::CONTENT_TYPE, "text/plain")
				.body(Body::from("Not Found"))
				.unwrap())
		}
	}
}

#[tokio::main]
pub async fn main() -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
	let make_svc = make_service_fn(|_conn| {
		async { Ok::<_, Infallible>(service_fn(hello)) }
	});
	let addr = ([127, 0, 0, 1], 8000).into();
	let server = Server::bind(&addr).serve(make_svc);
	println!("Listening on http://{}", addr);
	server.await?;
	Ok(())
}
