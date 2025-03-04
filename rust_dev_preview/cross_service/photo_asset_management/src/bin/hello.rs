use photo_asset_management::{
    common::{init_tracing_subscriber, Common},
    handlers::hello,
};

#[tokio::main]
async fn main() -> Result<(), lambda_runtime::Error> {
    init_tracing_subscriber();

    let common = Common::load_from_env().await;
    lambda_runtime::run(lambda_runtime::service_fn(|request| async {
        hello::handler(&common, request).await
    }))
    .await?;

    Ok(())
}
