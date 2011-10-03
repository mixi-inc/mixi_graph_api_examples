//
//  main.m
//  mixi-objc
//

#import <Foundation/Foundation.h>

#define kMixiApiUri @"http://api.mixi-platform.com/2/people/@me/@self"
#define kMixiUpgradeUri @"https://secure.mixi-platform.com/2/token"
#define kMixiUpgradeParamsFormat @"grant_type=authorization_code&client_id=%@&client_secret=%@&code=%@&redirect_uri=%@"
#define kMixiClientId @"YOUR CLIENT ID"
#define kMixiClientSecret @"YOUR CLIENT SECRET"
#define kMixiRedirectUri @"YOUR REDIRECT URI"

// JSONのパース？はいい加減なので参考にしないでください。
NSString* getAccessToken(NSString* json) {
    NSRange match = [json rangeOfString:@"\"access_token\":\""];
    if (match.location != NSNotFound) {
        match.location += match.length;
        match.length = 40;
        return [json substringWithRange:match];
    } else {
        return nil;
    }
}

int main (int argc, const char * argv[])
{

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    if (argc < 2) {
        NSLog(@"authcode is mandatory.");
        return 1;
    }
    NSString *code = [[[NSString alloc] initWithCString:argv[1] encoding:NSUTF8StringEncoding] autorelease];

    NSURL *url = [[[NSURL alloc] initWithString:kMixiUpgradeUri] autorelease];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
    request.HTTPMethod = @"POST";
    request.HTTPBody = [[NSString stringWithFormat:kMixiUpgradeParamsFormat, kMixiClientId, kMixiClientSecret, code, kMixiRedirectUri] dataUsingEncoding:NSUTF8StringEncoding];
    NSData *result = [NSURLConnection sendSynchronousRequest:request returningResponse:nil error:nil];
    NSString *json = [[[NSString alloc] initWithData:result encoding:NSUTF8StringEncoding] autorelease];
    NSString *accessToken = getAccessToken(json);

    url = [[[NSURL alloc] initWithString:kMixiApiUri] autorelease];
    request = [NSMutableURLRequest requestWithURL:url];
    [request setValue:[NSString stringWithFormat:@"OAuth %@", accessToken] forHTTPHeaderField:@"Authorization"];
    result = [NSURLConnection sendSynchronousRequest:request returningResponse:nil error:nil];
    json = [[[NSString alloc] initWithData:result encoding:NSUTF8StringEncoding] autorelease];
    NSLog(@"%@", json);
    
    [pool drain];
    return 0;
}

